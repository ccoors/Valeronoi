/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2024 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_ROBOT_API_SSE_H
#define VALERONOI_ROBOT_API_SSE_H

#include <QByteArray>
#include <QJsonDocument>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "../connection_configuration.h"

namespace Valeronoi::robot::api {

constexpr const std::chrono::milliseconds RECONNECT_TIMER{2000};

enum class ParserState { IDLE = 0, DATA };

class SSEConnection : public QObject {
  Q_OBJECT
 public:
  SSEConnection();

  void set_connection_configuration(const ConnectionConfiguration &conf);

  void set_initial_url(const QUrl &url);

  void set_url(const QUrl &url);

  void set_event(const QString &event);

  [[nodiscard]] QString current_data() const;

 public slots:
  void slot_connect();

  void slot_disconnect();

 signals:
  void signal_data_updated();

 private slots:
  void slot_ready_read();

  void slot_stream_finished();

  void slot_make_request();

 private:
  QNetworkRequest prepare_request(const QUrl &url) const;

  bool m_connected{false};
  QString m_event, m_data_buffer, m_initial_current_data, m_current_data,
      m_event_type;
  ParserState m_parser_state{ParserState::IDLE};
  QTimer m_reconnect_timer;
  ConnectionConfiguration m_connection_configuration;
  QUrl m_initial_url, m_url;
  QNetworkReply *m_reply{nullptr};
  QNetworkAccessManager m_qnam;
};

}  // namespace Valeronoi::robot::api

#endif
