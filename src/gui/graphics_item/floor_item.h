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
