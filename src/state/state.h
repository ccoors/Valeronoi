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
  std::string cls;
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
  std::unordered_map<std::string, Entity> entities;

  std::optional<Point> get_robot_position() const;
};

struct Measurement {
  int x, y;
  std::vector<double> data;
  double average;
};

typedef std::vector<Measurement> RawMeasurements;

struct MeasurementStatistics {
  int measurements, unique_places;
  double strongest, weakest;
};

enum class DISPLAY_MODE {
  Voronoi = 0,
  DataPoints = 1,
  None = 2
};  // Keep in sync with valeronoi.ui

struct DataSegment {
  int x{0}, y{0};
  double value;
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
