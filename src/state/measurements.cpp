#include "measurements.h"

namespace Valeronoi::state {

void Measurements::set_map(const RobotMap &map) { m_map = &map; }

void Measurements::slot_add_measurement(double data) {
  if (m_map != nullptr && m_map->is_valid()) {
    if (auto robot_position = m_map->get_map().get_robot_position()) {
      add_measurement(robot_position.value().x, robot_position.value().y, data);
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
    for (auto m : obj["data"].toArray()) {
      add_measurement(x, y, m.toDouble());
    }
  }
  emit signal_measurements_updated();
}

void Measurements::add_measurement(int x, int y, double value) {
  for (auto &d : m_data) {
    if (d.x == x && d.y == y) {
      d.data.push_back(value);
      double avg = 0;
      for (const auto &m : d.data) {
        avg += m;
      }
      d.average = avg / d.data.size();
      return;
    }
  }
  m_data.push_back(Measurement{x, y, {value}, value});
}

MeasurementStatistics Measurements::get_statistics() const {
  MeasurementStatistics ret{};
  ret.measurements = 0;
  ret.unique_places = m_data.size();
  ret.weakest = 0;
  ret.strongest = -1000;
  for (auto &m : m_data) {
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
