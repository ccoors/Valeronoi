#include "state.h"

namespace Valeronoi::state {

std::optional<Point> Map::get_robot_position() const {
  auto res = entities.find("robot_position");
  if (res == entities.end()) {
    return std::nullopt;
  }
  return std::optional(res->second.points[0]);
}

}  // namespace Valeronoi::state
