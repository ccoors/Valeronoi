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
#ifndef VALERONOI_GUI_GRAPHICS_ITEM_ENTITY_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_ENTITY_ITEM_H

#include "../../state/robot_map.h"
#include "map_based_item.h"

namespace Valeronoi::gui::graphics_item {
class EntityItem : public MapBasedItem {
 public:
  explicit EntityItem(const Valeronoi::state::RobotMap &robot_map,
                      QGraphicsItem *parent = nullptr);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

 private:
  static void paint_charger(QPainter *painter,
                            const Valeronoi::state::Entity &entity);

  static void paint_robot(QPainter *painter,
                          const Valeronoi::state::Entity &entity);
};

}  // namespace Valeronoi::gui::graphics_item

#endif
