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
#include "entity_item.h"

#include <QPen>
#include <algorithm>
#include <cmath>

namespace Valeronoi::gui::graphics_item {
EntityItem::EntityItem(const Valeronoi::state::RobotMap &robot_map,
                       QGraphicsItem *parent)
    : MapBasedItem(robot_map, parent) {}

void EntityItem::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {
  (void)widget;
  (void)option;
  if (m_robot_map.is_valid()) {
    const auto &map = m_robot_map.get_map();
    // Draw path(s) first
    std::for_each(
        map.entities.begin(), map.entities.end(), [=](const auto &entity) {
          if ((entity.type == "path" || entity.cls == "PathMapEntity") &&
              entity.points.size() > 1) {
            QPen pen;
            QColor path_color = Qt::white;
            path_color.setAlphaF(0.5);
            pen.setColor(path_color);
            pen.setWidth(2);
            painter->setPen(pen);
            painter->setBrush(Qt::transparent);
            QPainterPath path;
            path.moveTo(entity.points[0].x, entity.points[0].y);
            for (const auto &p : entity.points) {
              path.lineTo(p.x, p.y);
            }
            painter->drawPath(path);
          }
        });

    std::for_each(map.entities.begin(), map.entities.end(),
                  [=](const auto &entity) {
                    if (entity.points.empty()) {
                      return;
                    }
                    if (entity.type == "charger_location") {
                      paint_charger(painter, entity);
                    } else if (entity.type == "robot_position") {
                      paint_robot(painter, entity);
                    }
                  });
  }
}

void EntityItem::paint_robot(QPainter *painter,
                             const Valeronoi::state::Entity &entity) {
  QPen pen;
  pen.setColor(Qt::darkGray);
  pen.setWidthF(1.5);
  painter->setPen(pen);
  painter->setBrush(Qt::white);
  QPointF robot_center(entity.points[0].x, entity.points[0].y);
  painter->drawEllipse(robot_center, 10, 10);
  QPointF tower_head(entity.points[0].x, entity.points[0].y);
  tower_head += 3 * QPointF(std::cos(entity.angle), std::sin(entity.angle));
  pen.setWidthF(1);
  painter->setPen(pen);
  painter->drawEllipse(tower_head, 3, 3);
}

void EntityItem::paint_charger(QPainter *painter,
                               const Valeronoi::state::Entity &entity) {
  QPen pen;
  pen.setColor(QColor(50, 50, 50));
  pen.setWidthF(1.5);
  painter->setPen(pen);
  painter->setBrush(Qt::lightGray);
  QPointF charger_center(entity.points[0].x, entity.points[0].y);
  painter->drawEllipse(charger_center, 12, 12);

  QPainterPath path;
  path.moveTo(charger_center.x() + 2, charger_center.y() - 6);
  path.lineTo(charger_center.x() - 3.5, charger_center.y() + 1);
  path.lineTo(charger_center.x() - 1, charger_center.y() + 1);
  path.lineTo(charger_center.x() - 2, charger_center.y() + 6);

  path.lineTo(charger_center.x() + 3.5, charger_center.y() - 1);
  path.lineTo(charger_center.x() + 1, charger_center.y() - 1);
  path.closeSubpath();

  painter->setPen(
      QPen(QColor(0, 120, 255), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
  painter->setBrush(Qt::white);
  painter->drawPath(path);
}

}  // namespace Valeronoi::gui::graphics_item
