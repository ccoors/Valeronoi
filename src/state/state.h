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
#ifndef VALERONOI_STATE_STATE_H
#define VALERONOI_STATE_STATE_H

#include <QColor>
#include <QMetaType>
#include <QPolygon>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Valeronoi::state {

struct Point {
  int x, y;
};

struct Block {
  int x, y;
};

inline bool operator==(const Block &lhs, const Block &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator<(const Block &lhs, const Block &rhs) {
  return (lhs.x + lhs.y) < (rhs.x + rhs.y);
}

struct Entity {
  std::string type, cls;
  double angle;
  std::vector<Point> points;
};

struct Layer {
  std::vector<Block> blocks;
};

struct Map {
  int pixel_size;
  int size_x, size_y;
  int crop_x, crop_y;
  std::unordered_map<std::string, Layer> layers;
  std::vector<Entity> entities;

  std::optional<Point> get_robot_position() const;
};

struct Measurement {
  int x, y;
  int wifi_id;
  std::vector<double> data;
  double average;
};

typedef std::vector<Measurement> RawMeasurements;

struct MeasurementStatistics {
  int measurements, unique_places, unique_wifi_APs;
  double strongest, weakest;
};

enum class DISPLAY_MODE {
  Voronoi = 0,
  DataPoints = 1,
  None = 2
};  // Keep in sync with valeronoi.ui

struct DataSegment {
  int x{0}, y{0};
  double value{0.0};
  QPolygon polygon;
  QColor color;
};

typedef QList<Valeronoi::state::DataSegment> DataSegments;

}  // namespace Valeronoi::state

namespace std {
template <>
struct hash<Valeronoi::state::Block> {
  std::size_t operator()(Valeronoi::state::Block const &s) const noexcept {
    std::size_t h1 = std::hash<int>{}(s.x);
    std::size_t h2 = std::hash<int>{}(s.y);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace std

#ifndef Q_DECLARE_METATYPE
// To make cppcheck happy
#define Q_DECLARE_METATYPE(TYPE)  // TYPE
#endif
Q_DECLARE_METATYPE(Valeronoi::state::DataSegments)

#endif
