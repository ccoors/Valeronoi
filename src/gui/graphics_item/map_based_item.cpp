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
