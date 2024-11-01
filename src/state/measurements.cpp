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
#include "measurements.h"

namespace Valeronoi::state {

void Measurements::set_map(const RobotMap &map) { m_map = &map; }

void Measurements::slot_add_measurement(double signal, int wifi_id) {
  if (m_map != nullptr && m_map->is_valid()) {
    if (auto robot_position = m_map->get_map().get_robot_position()) {
      add_measurement(robot_position.value().x, robot_position.value().y,
                      signal, wifi_id);
      emit signal_measurements_updated();
    } else {
      qDebug() << "Could not find robot on map";
    }
  }
}

QJsonArray Measurements::get_json() const {
  QJsonArray arr;
  for (const auto &d : m_data) {
    auto obj = QJsonObject();
    obj.insert("x", d.x);
    obj.insert("y", d.y);
    obj.insert("wifi", d.wifi_id);
    auto data = QJsonArray();
    for (auto v : d.data) {
      data.append(v);
    }
    obj.insert("data", data);
    arr.append(obj);
  }
  return arr;
}

void Measurements::reset() {
  m_data.clear();
  emit signal_measurements_updated();
}

void Measurements::set_json(const QJsonArray &json) {
  m_data.clear();
  for (auto v : json) {
    const auto obj = v.toObject();
    int x = obj["x"].toInt();
    int y = obj["y"].toInt();
    int wifiId = unknown_wifi_id;
    if (obj.contains("wifi")) {
      wifiId = obj["wifi"].toInt(unknown_wifi_id);
    }
    for (auto m : obj["data"].toArray()) {
      add_measurement(x, y, m.toDouble(), wifiId);
    }
  }
  emit signal_measurements_updated();
}

void Measurements::add_measurement(int x, int y, double value, int wifi_id) {
  for (auto &d : m_data) {
    if (d.x == x && d.y == y && d.wifi_id == wifi_id) {
      d.data.push_back(value);
      double avg = 0;
      for (const auto &m : d.data) {
        avg += m;
      }
      d.average = avg / d.data.size();
      return;
    }
  }
  m_data.push_back(Measurement{x, y, wifi_id, {value}, value});
}

MeasurementStatistics Measurements::get_statistics() const {
  MeasurementStatistics ret{};
  ret.measurements = 0;
  ret.unique_places = m_data.size();
  ret.weakest = 0;
  ret.strongest = -1000;
  ret.unique_wifi_APs = 0;
  QVector<int> temp_wifi_count;
  for (auto &m : m_data) {
    if (!temp_wifi_count.contains(m.wifi_id)) {
      temp_wifi_count.push_back(m.wifi_id);
      ret.unique_wifi_APs++;
    }
    for (auto &d : m.data) {
      ret.measurements++;
      ret.weakest = std::min(ret.weakest, d);
      ret.strongest = std::max(ret.strongest, d);
    }
  }
  return ret;
}

const RawMeasurements &Measurements::get_measurements() const { return m_data; }

}  // namespace Valeronoi::state
