/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_GUI_GRAPHICS_ITEM_MAP_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_MAP_ITEM_H

#ifndef QT_NO_CONTEXTMENU
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#endif

#include <functional>

#include "../../state/robot_map.h"
#include "map_based_item.h"

namespace Valeronoi::gui::graphics_item {
class MapItem : public MapBasedItem {
 public:
  explicit MapItem(const Valeronoi::state::RobotMap &robot_map,
                   std::function<void(int, int)> relocate,
                   QGraphicsItem *parent = nullptr);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void set_wall_color(QColor color);

#ifndef QT_NO_CONTEXTMENU
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
#endif

 private:
  QFont m_font;
  QColor m_wall_color;
  std::function<void(int, int)>
      m_relocate;  // This is a bit hacky, but since MapItem is not a QObject,
                   // we can't use signals/slots
};

}  // namespace Valeronoi::gui::graphics_item

#endif
