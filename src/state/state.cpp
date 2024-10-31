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
#include "state.h"

#include <algorithm>

namespace Valeronoi::state {

std::optional<Point> Map::get_robot_position() const {
  auto res = std::find_if(entities.begin(), entities.end(), [=](const auto &e) {
    return e.type == "robot_position";
  });
  if (res == entities.end()) {
    return std::nullopt;
  }
  return {res->points[0]};
}

}  // namespace Valeronoi::state
