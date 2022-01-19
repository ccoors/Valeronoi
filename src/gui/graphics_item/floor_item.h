/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2022 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_GUI_GRAPHICS_ITEM_FLOOR_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_FLOOR_ITEM_H

#include <QPainterPath>

#include "../../state/robot_map.h"
#include "map_based_item.h"

namespace Valeronoi::gui::graphics_item {
class FloorItem : public MapBasedItem {
 public:
  explicit FloorItem(const Valeronoi::state::RobotMap &robot_map,
                     QGraphicsItem *parent = nullptr);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void set_floor_color(QColor color);

  void map_updated() override;

  [[nodiscard]] const QPainterPath &get_floor_path() const;

 private:
  QColor m_floor_color;

  QPainterPath m_floor_path;
};

}  // namespace Valeronoi::gui::graphics_item

#endif
