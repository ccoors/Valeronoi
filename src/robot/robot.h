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
#ifndef VALERONOI_ROBOT_ROBOT_H
#define VALERONOI_ROBOT_ROBOT_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include "api/valetudo_v2.h"
#include "commands.h"
#include "connection_configuration.h"
#include "robot_information.h"

namespace Valeronoi::robot {

class Robot : public QObject {
  Q_OBJECT
 public:
  Robot();

  void set_connection_configuration(const ConnectionConfiguration &conf);

  [[nodiscard]] const RobotInformation *get_information() const;

  [[nodiscard]] QString get_error() const;

  [[nodiscard]] bool is_connected() const;

  [[nodiscard]] bool is_connecting() const;

  [[nodiscard]] QString get_map_data() const;

  void send_command(BASIC_COMMANDS command);

 public slots:
  void slot_connect();

  void slot_disconnect();

  void slot_subscribe_wifi(double interval);

  void slot_stop_wifi_subscription();

  void slot_relocate(int x, int y);

 signals:
  void signal_connecting();

  void signal_connecting_step(float value);  // 0.0 - 1.0

  void signal_connected();

  void signal_connection_error();

  void signal_connection_ended();

  void signal_map_updated();

  void signal_wifi_updated(double value);

 private slots:
  void slot_get_wifi();

 private:
  QTimer m_wifi_timer;
  Valeronoi::robot::api::v2::ValetudoAPI m_api;
};

}  // namespace Valeronoi::robot

#endif
