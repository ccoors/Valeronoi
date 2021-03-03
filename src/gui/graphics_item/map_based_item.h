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
