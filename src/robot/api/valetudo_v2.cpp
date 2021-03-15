/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021 Christian Friedrich Coors <me@ccoors.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "valetudo_v2.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

namespace Valeronoi::robot::api::v2 {

const QUrl VALETUDO_VERSION = QUrl("/api/v2/valetudo/version");
const QUrl ROBOT_INFO = QUrl("/api/v2/robot");
const QUrl ROBOT_STATE = QUrl("/api/v2/robot/state");
const QUrl ROBOT_CAPABILITIES_INFO = QUrl("/api/v2/robot/capabilities");
const QUrl ROBOT_MAP_SSE = QUrl("/api/v2/robot/state/map/sse");

const QList<QUrl> ROBOT_INIT_URLS = {VALETUDO_VERSION, ROBOT_INFO, ROBOT_STATE,
                                     ROBOT_CAPABILITIES_INFO};
const QStringList SUPPORTED_VALETUDO_VERSIONS = {"2021.02.0", "2021.03.0"};

ValetudoAPI::ValetudoAPI() {
  m_map_connection.set_url(ROBOT_MAP_SSE);
  m_map_connection.set_event("MapUpdated");
  connect(&m_map_connection,
          &Valeronoi::robot::api::SSEConnection::signal_data_updated, this,
          &ValetudoAPI::slot_map_updated);
}

void ValetudoAPI::set_connection_configuration(
    const ConnectionConfiguration &conf) {
  m_connection_configuration = conf;
  m_map_connection.set_connection_configuration(conf);
}

const RobotInformation *ValetudoAPI::get_information() const {
  return &m_robot_information;
}

QString ValetudoAPI::get_error() const { return m_error_message; }

QString ValetudoAPI::get_warning() const { return m_warning_message; }

bool ValetudoAPI::is_connected() const { return m_connected; }

bool ValetudoAPI::is_connecting() const { return m_connecting; }

void ValetudoAPI::slot_connect() {
  if (m_connected || m_connecting) {
    return;
  }
  m_connecting = true;
  m_connecting_step = 0;
  m_connection_responses.clear();
  m_error_message = "";
  m_warning_message = "";
  emit signal_connecting();
  next_connection_step();
}

void ValetudoAPI::slot_disconnect() {
  if (!m_connected && !m_connecting) {
    return;
  }
  m_connected = false;
  m_connecting = false;
  m_map_connection.slot_disconnect();
  emit signal_connection_ended();
}

void ValetudoAPI::next_connection_step() {
  if (m_connecting_step == ROBOT_INIT_URLS.size()) {
    // We're done
    m_connecting = false;
    m_connected = true;

    // Parse responses from connection sequence
    const auto valetudo_version =
        m_connection_responses[0].object()["release"].toString();
    m_robot_information.m_valetudo_version = valetudo_version;
    if (!SUPPORTED_VALETUDO_VERSIONS.contains(
            m_robot_information.m_valetudo_version)) {
      m_warning_message = QString("Unknown Valetudo Version: %1")
                              .arg(m_robot_information.m_valetudo_version);
    }

    const auto robot_info = m_connection_responses[1].object();
    m_robot_information.m_manufacturer = robot_info["manufacturer"].toString();
    m_robot_information.m_model_name = robot_info["modelName"].toString();
    m_robot_information.m_implementation =
        robot_info["implementation"].toString();

    const auto robot_state = m_connection_responses[2].object();
    m_robot_information.m_attributes = robot_state["attributes"].toArray();

    const auto robot_capabilities = m_connection_responses[3].array();
    m_robot_information.m_capabilities.clear();
    for (const auto &&c : robot_capabilities) {
      m_robot_information.m_capabilities.push_back(c.toString());
    }

    m_map_connection.slot_connect();
    emit signal_connected();
    return;
  }

  emit signal_connecting_step(static_cast<float>(m_connecting_step) /
                              ROBOT_INIT_URLS.size());
  const auto next_url = ROBOT_INIT_URLS[m_connecting_step];
  auto request =
      QNetworkRequest(m_connection_configuration.m_url.resolved(next_url));
  m_connection_configuration.prepare_request(request);
  auto reply = m_qnam.get(request);
  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (m_connecting) {
      if (reply->error() != QNetworkReply::NetworkError::NoError) {
        m_error_message = reply->errorString();
        emit signal_connection_error();
        slot_disconnect();
      } else {
        auto resp_data = reply->readAll();
        QJsonParseError error;
        auto json = QJsonDocument::fromJson(resp_data, &error);
        if (json.isNull()) {
          // Invalid JSON received
          m_error_message = error.errorString();
          emit signal_connection_error();
          slot_disconnect();
        }
        m_connection_responses.push_back(json);
        next_connection_step();
      }
    }
    reply->deleteLater();
  });

  m_connecting_step++;
}

QNetworkReply *ValetudoAPI::request(const QString &verb, const QUrl &url,
                                    bool disconnect_on_failure, bool gc,
                                    const QByteArray *data) {
  if (!m_connected) {
    return nullptr;
  }
  auto request =
      QNetworkRequest(m_connection_configuration.m_url.resolved(url));
  m_connection_configuration.prepare_request(request);
  if (data != nullptr) {
    request.setRawHeader("Content-Type", "application/json");
  }
  auto response = data != nullptr
                      ? m_qnam.sendCustomRequest(request, verb.toUtf8(), *data)
                      : m_qnam.sendCustomRequest(request, verb.toUtf8());
  if (disconnect_on_failure) {
    connect(response, &QNetworkReply::errorOccurred, this,
            [=](QNetworkReply::NetworkError error) {
              if (error != QNetworkReply::NetworkError::NoError) {
                m_error_message = response->errorString();
                emit signal_connection_error();
                slot_disconnect();
              }
            });
  }
  if (gc) {
    connect(response, &QNetworkReply::finished, this,
            [=]() { response->deleteLater(); });
  }
  return response;
}

void ValetudoAPI::slot_map_updated() { emit signal_map_updated(); }

QString ValetudoAPI::get_map_data() const {
  return m_map_connection.current_data();
}

static QJsonDocument gen_document(const QString &action) {
  auto ret = QJsonDocument();
  auto object = QJsonObject();
  object.insert("action", action);
  ret.setObject(object);
  return ret;
}

void ValetudoAPI::send_command(Valeronoi::robot::BASIC_COMMANDS command) {
  if (!m_connected) {
    return;
  }
  switch (command) {
    case BASIC_COMMANDS::START: {
      auto data = gen_document("start").toJson();
      request("PUT", QUrl("/api/v2/robot/capabilities/BasicControlCapability"),
              false, true, &data);
    } break;
    case BASIC_COMMANDS::PAUSE: {
      auto data = gen_document("pause").toJson();
      request("PUT", QUrl("/api/v2/robot/capabilities/BasicControlCapability"),
              false, true, &data);
    } break;
    case BASIC_COMMANDS::STOP: {
      auto data = gen_document("stop").toJson();
      request("PUT", QUrl("/api/v2/robot/capabilities/BasicControlCapability"),
              false, true, &data);
    } break;
    case BASIC_COMMANDS::HOME: {
      auto data = gen_document("home").toJson();
      request("PUT", QUrl("/api/v2/robot/capabilities/BasicControlCapability"),
              false, true, &data);
    } break;
    case BASIC_COMMANDS::LOCATE: {
      // TODO: this is not a BasicControlCapability, but we assume it's present
      // on every robot
      auto data = gen_document("locate").toJson();
      request("PUT", QUrl("/api/v2/robot/capabilities/LocateCapability"), false,
              true, &data);
    }
  }
}

void ValetudoAPI::relocate(int x, int y) {
  if (!m_connected) {
    return;
  }
  auto document = QJsonDocument();
  auto object = QJsonObject();
  object.insert("action", "goto");
  auto coords = QJsonObject();
  coords.insert("x", x);
  coords.insert("y", y);
  object.insert("coordinates", coords);
  document.setObject(object);
  auto data = document.toJson();
  request("PUT", QUrl("/api/v2/robot/capabilities/GoToLocationCapability"),
          false, true, &data);
}

}  // namespace Valeronoi::robot::api::v2
