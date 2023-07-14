/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2023 Christian Friedrich Coors <me@ccoors.de>
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
#include "robot.h"

#include "../gui/dialog/robot_config.h"

namespace Valeronoi::robot {

Robot::Robot() {
  // Reroute signals from API
  connect(&m_api, &Valeronoi::robot::api::v2::ValetudoAPI::signal_connecting,
          this, [=]() { emit signal_connecting(); });
  connect(&m_api,
          &Valeronoi::robot::api::v2::ValetudoAPI::signal_connecting_step, this,
          [=](float value) { emit signal_connecting_step(value); });
  connect(&m_api, &Valeronoi::robot::api::v2::ValetudoAPI::signal_connected,
          this, [=]() { emit signal_connected(); });
  connect(&m_api,
          &Valeronoi::robot::api::v2::ValetudoAPI::signal_connection_error,
          this, [=]() { emit signal_connection_error(); });
  connect(&m_api,
          &Valeronoi::robot::api::v2::ValetudoAPI::signal_connection_ended,
          this, [=]() { emit signal_connection_ended(); });
  connect(&m_api, &Valeronoi::robot::api::v2::ValetudoAPI::signal_map_updated,
          this, [=]() { emit signal_map_updated(); });
  connect(&m_wifi_timer, &QTimer::timeout, this, &Robot::slot_get_wifi);
  m_wifi_timer.setTimerType(Qt::CoarseTimer);
  m_wifi_timer.setSingleShot(false);
}

void Robot::set_connection_configuration(const ConnectionConfiguration &conf) {
  m_api.set_connection_configuration(conf);
}

QString Robot::get_error() const { return m_api.get_error(); }

bool Robot::is_connected() const { return m_api.is_connected(); }

bool Robot::is_connecting() const { return m_api.is_connecting(); }

const RobotInformation *Robot::get_information() const {
  return m_api.get_information();
}

void Robot::slot_connect() {
  m_current_wifi_connection = WifiInformation();
  m_api.slot_connect();
}

void Robot::slot_disconnect() {
  m_api.slot_disconnect();
  m_current_wifi_connection = WifiInformation();
}

void Robot::slot_subscribe_wifi(double interval) {
  m_wifi_timer.start(static_cast<int>(1000 * interval));
}

void Robot::slot_stop_wifi_subscription() { m_wifi_timer.stop(); }

void Robot::slot_get_wifi() {
  if (is_connected()) {
    qDebug() << "Making WiFi Request";
    auto reply =
        m_api.request("GET", Valeronoi::robot::api::v2::ROBOT_WIFI_CAPABILITY);
    connect(reply, &QNetworkReply::finished, this, [=]() {
      auto resp_data = reply->readAll();
      reply->deleteLater();
      QJsonParseError error{};
      const auto json = QJsonDocument::fromJson(resp_data, &error);
      if (json.isNull()) {
        qDebug() << "Invalid JSON:" << error.errorString();
        return;
      }

      const auto response_class = json.object()["__class"].toString();
      if (response_class == "ValetudoWifiStatus" ||
          response_class == "ValetudoWifiConfiguration") {
        const QJsonObject details_object = json.object()["details"].toObject();
        const WifiInformation wifi_info(details_object);
        if (wifi_info.bssid() != m_current_wifi_connection.bssid()) {
            m_current_wifi_connection = wifi_info;
            emit signal_current_wifi_updated(wifi_info);
        }
        if (wifi_info.has_valid_signal()) {
            emit signal_wifi_info_updated(wifi_info);
        } else {
          qDebug()
              << "Message did not contain a valid signal strength, ignoring";
        }
      } else {
        qDebug().nospace() << "Received unexpected JSON object of class '"
                           << response_class << "', ignoring";
      }
    });
  }
}

QString Robot::get_map_data() const { return m_api.get_map_data(); }

void Robot::send_command(BASIC_COMMANDS command) {
  m_api.send_command(command);
}

void Robot::slot_relocate(int x, int y) { m_api.relocate(x, y); }

}  // namespace Valeronoi::robot
