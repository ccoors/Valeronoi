/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_CLI_HEADLESS_RECORDER_H
#define VALERONOI_CLI_HEADLESS_RECORDER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "../robot/robot.h"
#include "../state/measurements.h"
#include "../state/robot_map.h"

namespace Valeronoi::cli {

class HeadlessRecorder : public QObject {
  Q_OBJECT

 public:
  explicit HeadlessRecorder(QObject* parent = nullptr);

  void set_output(const QString& path);
  void set_duration(int seconds);
  void set_interval(double seconds);
  void set_url(const QString& url);
  void set_auth(const QString& username, const QString& password);
  void set_command(const QString& command);
  void set_operation_mode(const QString& mode);
  void set_return_home(bool enabled);
  void load_file(const QString& path);

  int run();

 private slots:
  void slot_duration_elapsed();
  void slot_connected();
  void slot_connection_error();
  void slot_map_updated();
  void slot_wifi_info_updated(Valeronoi::robot::WifiInformation wifi_info);

 private:
  void save_and_exit(int code);

  Valeronoi::robot::Robot m_robot;
  Valeronoi::state::RobotMap m_robot_map;
  Valeronoi::state::Measurements m_measurements;

  QString m_output_path;
  QString m_load_path;
  QString m_url;
  QString m_username;
  QString m_password;
  QString m_command;
  QString m_operation_mode;
  bool m_auth{false};
  bool m_return_home{false};

  int m_duration_seconds{0};
  double m_interval_seconds{1.0};

  QTimer m_duration_timer;
  bool m_exiting{false};
};

}  // namespace Valeronoi::cli

#endif  // VALERONOI_CLI_HEADLESS_RECORDER_H
