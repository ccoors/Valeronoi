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
#ifndef VALERONOI_STATE_ROBOT_MAP_H
#define VALERONOI_STATE_ROBOT_MAP_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include "state.h"

namespace Valeronoi::state {

class RobotMap : public QObject {
  Q_OBJECT
 public:
  explicit RobotMap(QObject *parent = nullptr);

  void update_map_json(const QString &json);

  void update_map_json(const QJsonObject &json);

  void reset();

  [[nodiscard]] bool is_valid() const;

  [[nodiscard]] QString error_msg() const;

  [[nodiscard]] const Map &get_map() const;

  [[nodiscard]] const QJsonObject &get_map_json() const;

 signals:
  void signal_map_updated();

 private:
  void generate_map();

  QJsonObject m_map_json{};
  int m_map_version{0};
  bool m_valid{false};
  QString m_error;
  Map m_map;
};
}  // namespace Valeronoi::state

#endif
