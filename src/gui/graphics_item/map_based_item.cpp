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
#include "map_based_item.h"

namespace Valeronoi::gui::graphics_item {
MapBasedItem::MapBasedItem(const Valeronoi::state::RobotMap &robot_map,
                           QGraphicsItem *parent)
    : QGraphicsItem(parent), m_robot_map(robot_map) {}

QRectF MapBasedItem::boundingRect() const { return m_map_bounds; }

void MapBasedItem::map_updated() {
  QRectF new_bounds;
  if (m_robot_map.is_valid()) {
    new_bounds = QRectF(0, 0, m_robot_map.get_map().size_x,
                        m_robot_map.get_map().size_y);
  } else {
    new_bounds = QRectF(0, 0, 400.0, 50.0);
  }
  if (new_bounds != m_map_bounds) {
    m_map_bounds = new_bounds;
    prepareGeometryChange();
  }
  update();
}

}  // namespace Valeronoi::gui::graphics_item
