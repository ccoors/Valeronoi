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
#include "map_item.h"

#include <utility>

namespace Valeronoi::gui::graphics_item {
MapItem::MapItem(const Valeronoi::state::RobotMap &robot_map,
                 std::function<void(int, int)> relocate, QGraphicsItem *parent)
    : MapBasedItem(robot_map, parent),
      m_font("Source Code Pro", 12),
      m_relocate(std::move(relocate)) {}

void MapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                    QWidget *widget) {
  (void)option;
  (void)widget;
  painter->setPen(Qt::transparent);
  painter->setFont(m_font);

  if (m_robot_map.is_valid()) {
    const auto &map = m_robot_map.get_map();

    painter->setBrush(m_wall_color);
    painter->setRenderHints(
        QPainter::Antialiasing,
        false);  // Antialiasing the walls leads to a thin grid
    const auto walls = map.layers.find("wall");
    if (walls != map.layers.end()) {
      for (const auto &block : walls->second.blocks) {
        painter->drawRect(block.x, block.y, map.pixel_size, map.pixel_size);
      }
    }
  } else {
    painter->setBrush(Qt::black);
    painter->drawRect(0, 0, 400, 50);
    painter->setPen(Qt::lightGray);
    painter->drawText(0, 0, 400, 50,
                      Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                      m_robot_map.error_msg());
  }
}

void MapItem::set_wall_color(QColor color) {
  m_wall_color = color;
  update();
}

#ifndef QT_NO_CONTEXTMENU
void MapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  const auto &map = m_robot_map.get_map();
  QMenu menu;
  QAction *go_to_action = menu.addAction(QObject::tr("Relocate here"));
  if (!m_robot_map.is_valid()) {
    go_to_action->setEnabled(false);
  }
  QAction *selected_action = menu.exec(event->screenPos());
  if (selected_action == go_to_action) {
    m_relocate(event->pos().x() + map.crop_x, event->pos().y() + map.crop_y);
  }
}
#endif

}  // namespace Valeronoi::gui::graphics_item
