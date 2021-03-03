#include "floor_item.h"

namespace Valeronoi::gui::graphics_item {
FloorItem::FloorItem(const Valeronoi::state::RobotMap &robot_map,
                     QGraphicsItem *parent)
    : MapBasedItem(robot_map, parent) {}

void FloorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {
  (void)option;
  (void)widget;
  painter->setPen(Qt::transparent);

  // Anti aliasing the floor leads to a thin grid
  painter->setRenderHints(QPainter::Antialiasing, false);

  painter->setBrush(m_floor_color);
  painter->drawPath(m_floor_path);
}

void FloorItem::set_floor_color(QColor color) {
  m_floor_color = color;
  update();
}

void FloorItem::map_updated() {
  m_floor_path.clear();
  m_floor_path.setFillRule(Qt::WindingFill);
  if (m_robot_map.is_valid()) {
    const auto &map = m_robot_map.get_map();
    const auto floor = map.layers.find("floor");
    if (floor != map.layers.end()) {
      for (const auto &block : floor->second.blocks) {
        m_floor_path.addRect(block.x, block.y, map.pixel_size, map.pixel_size);
      }
    }
  }
  MapBasedItem::map_updated();
}

const QPainterPath &FloorItem::get_floor_path() const { return m_floor_path; }

}  // namespace Valeronoi::gui::graphics_item
