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
#ifndef VALERONOI_STATE_MEASUREMENTS_H
#define VALERONOI_STATE_MEASUREMENTS_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <vector>

#include "robot_map.h"
#include "state.h"

namespace Valeronoi::state {

class Measurements : public QObject {
  Q_OBJECT
 public:
  void reset();

  void set_json(const QJsonArray &json);

  void set_map(const RobotMap &map);

  [[nodiscard]] const RawMeasurements &get_measurements() const;

  [[nodiscard]] QJsonArray get_json() const;

  [[nodiscard]] MeasurementStatistics get_statistics() const;

  int unkown_wifi_id = 0;

 signals:
  void signal_measurements_updated();

 public slots:
  void slot_add_measurement(double signal, int wifi_id);

 private:
  void add_measurement(int x, int y, double value, int wifi_id);

  const RobotMap *m_map{nullptr};

  std::vector<Measurement> m_data;
};

}  // namespace Valeronoi::state

#endif
