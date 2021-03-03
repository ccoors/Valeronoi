#ifndef VALERONOI_GUI_WIDGET_DISPLAY_WIDGET_H
#define VALERONOI_GUI_WIDGET_DISPLAY_WIDGET_H

#include <QColor>
#include <QGraphicsView>
#include <QPainter>
#include <QWheelEvent>
#include <QWidget>

#include "../../state/measurements.h"
#include "../../state/robot_map.h"
#include "../../util/colormap.h"
#include "../../util/segment_generator.h"
#include "../graphics_item/entity_item.h"
#include "../graphics_item/floor_item.h"
#include "../graphics_item/map_item.h"
#include "../graphics_item/measurement_item.h"

namespace Valeronoi::gui::widget {

class DisplayWidget : public QGraphicsView {
  Q_OBJECT
 public:
  explicit DisplayWidget(const Valeronoi::state::RobotMap &robot_map,
                         const Valeronoi::state::Measurements &measurements,
                         QWidget *parent);

  [[nodiscard]] QColor get_background_color() const;

  void set_background_color(QColor color);

  [[nodiscard]] QColor get_wall_color() const;

  void set_wall_color(QColor color);

  void set_color_map(const Valeronoi::util::RGBColorMap *color_map);

  void set_floor(bool enabled, QColor color);

  [[nodiscard]] bool get_floor_enabled() const;

  [[nodiscard]] QColor get_floor_color() const;

  void set_entities(bool enabled);

  [[nodiscard]] bool get_entities_enabled() const;

  [[nodiscard]] qreal get_zoom_factor() const;

  void set_opengl(bool enabled);

  [[nodiscard]] bool get_opengl() const;

  void set_antialiasing(bool enabled);

  [[nodiscard]] bool get_antialiasing() const;

  void set_restrict_floor(bool enabled);

  [[nodiscard]] bool get_restrict_floor() const;

  void set_restrict_path(bool enabled);

  [[nodiscard]] bool get_restrict_path() const;

  [[nodiscard]] int get_simplify() const;

 signals:
  void signal_relocate(int x, int y);

 public slots:
  void slot_map_updated();

  void slot_measurements_updated();

  void slot_set_display_mode(int display_mode);

  void slot_update_map_rect();

  void slot_set_simplify(int value);

 protected:
  void wheelEvent(QWheelEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

 private:
  void zoom_by(qreal factor);

  QColor m_background_color, m_wall_color, m_floor_color;
  bool m_draw_floor{true}, m_draw_entities{true}, m_use_opengl{false},
      m_antialiasing{true}, m_restrict_floor{true}, m_restrict_path{true};
  int m_simplify{1};
  const Valeronoi::util::RGBColorMap *m_color_map{nullptr};
  Valeronoi::util::SegmentGenerator m_segment_generator;

  Valeronoi::state::DISPLAY_MODE m_display_mode{
      Valeronoi::state::DISPLAY_MODE::Voronoi};

  const Valeronoi::state::Measurements &m_measurements;

  Valeronoi::gui::graphics_item::MapItem *m_map_item;
  Valeronoi::gui::graphics_item::FloorItem *m_floor_item;
  Valeronoi::gui::graphics_item::EntityItem *m_entity_item;
  Valeronoi::gui::graphics_item::MeasurementItem *m_measurement_item;

  QPainterPath m_floor_path;
};

}  // namespace Valeronoi::gui::widget

#endif
