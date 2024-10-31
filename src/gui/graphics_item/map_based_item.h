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
#ifndef VALERONOI_GUI_GRAPHICS_ITEM_MAP_BASED_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_MAP_BASED_ITEM_H

#include <QColor>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QJsonDocument>
#include <QPainter>

#include "../../state/robot_map.h"

namespace Valeronoi::gui::graphics_item {

class MapBasedItem : public QGraphicsItem {
 public:
  explicit MapBasedItem(const Valeronoi::state::RobotMap &robot_map,
                        QGraphicsItem *parent = nullptr);

  [[nodiscard]] QRectF boundingRect() const override;

  virtual void map_updated();

 protected:
  const Valeronoi::state::RobotMap &m_robot_map;

  QRectF m_map_bounds;
};

}  // namespace Valeronoi::gui::graphics_item

#endif
