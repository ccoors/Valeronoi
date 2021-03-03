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
