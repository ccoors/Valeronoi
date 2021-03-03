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
