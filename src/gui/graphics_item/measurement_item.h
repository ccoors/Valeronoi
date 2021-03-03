#ifndef VALERONOI_GUI_GRAPHICS_ITEM_MEASUREMENT_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_MEASUREMENT_ITEM_H

#include <QFont>
#include <QPicture>

#include "../../state/measurements.h"
#include "../../state/robot_map.h"
#include "../../state/state.h"
#include "../../util/colormap.h"
#include "map_based_item.h"

namespace Valeronoi::gui::graphics_item {

class MeasurementItem : public MapBasedItem {
 public:
  explicit MeasurementItem(const Valeronoi::state::RobotMap &robot_map,
                           QGraphicsItem *parent = nullptr);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  void set_data_segments(const Valeronoi::state::DataSegments &segments);

  void set_display_mode(Valeronoi::state::DISPLAY_MODE display_mode);

  void set_color_map(const Valeronoi::util::RGBColorMap *color_map);

  void set_restrict_path(bool enabled);

  void set_restrict_path(const QPainterPath &path);

  void set_restrict_points(bool enabled);

  [[nodiscard]] QRectF boundingRect() const override;

 private:
  void calculate_colors();

  [[nodiscard]] QColor get_color(double value) const;

  [[nodiscard]] QColor color_value(double normalized_value) const;

  double m_min{0.0}, m_max{0.0};

  const Valeronoi::util::RGBColorMap *m_color_map{nullptr};

  Valeronoi::state::DISPLAY_MODE m_display_mode{
      Valeronoi::state::DISPLAY_MODE::Voronoi};

  Valeronoi::state::DataSegments m_data_segments;

  bool m_restrict_path{true}, m_restrict_points{true};
  QPainterPath m_path, m_points_path;
  QFont m_font;
  QPicture m_legend;
};

}  // namespace Valeronoi::gui::graphics_item

#endif
