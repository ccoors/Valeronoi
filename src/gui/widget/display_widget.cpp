/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2023 Christian Friedrich Coors <me@ccoors.de>
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
#include "display_widget.h"

#include <QSettings>
#include <QtMath>
#ifndef QT_NO_OPENGL
#include <QOpenGLWidget>
#endif

namespace Valeronoi::gui::widget {

DisplayWidget::DisplayWidget(const Valeronoi::state::RobotMap &robot_map,
                             const Valeronoi::state::Measurements &measurements,
                             QWidget *parent)
    : QGraphicsView(parent), m_measurements{measurements} {
  setScene(new QGraphicsScene(this));
  setTransformationAnchor(AnchorUnderMouse);
  setDragMode(ScrollHandDrag);
  setViewportUpdateMode(FullViewportUpdate);

  m_map_item = new Valeronoi::gui::graphics_item::MapItem(
      robot_map, [=](int x, int y) { emit signal_relocate(x, y); });
  scene()->addItem(m_map_item);
  m_floor_item = new Valeronoi::gui::graphics_item::FloorItem(robot_map);
  scene()->addItem(m_floor_item);
  m_entity_item = new Valeronoi::gui::graphics_item::EntityItem(robot_map);
  scene()->addItem(m_entity_item);
  m_measurement_item =
      new Valeronoi::gui::graphics_item::MeasurementItem(robot_map);
  scene()->addItem(m_measurement_item);

  m_floor_item->setZValue(0.0);
  m_measurement_item->setZValue(10.0);
  m_map_item->setZValue(20.0);
  m_entity_item->setZValue(30.0);

  QSettings settings;
  m_background_color =
      settings.value("display/backgroundColor", QColor(20, 20, 20))
          .value<QColor>();
  m_wall_color = settings.value("display/wallColor", QColor(180, 180, 180))
                     .value<QColor>();
  m_floor_color =
      settings.value("display/floorColor", QColor(0, 77, 112)).value<QColor>();
  m_draw_floor = settings.value("display/drawFloor", true).toBool();
  m_draw_entities = settings.value("display/drawEntities", true).toBool();
  m_use_opengl = settings.value("display/useOpenGL", false).toBool();
  m_antialiasing = settings.value("display/antialiasing", true).toBool();
  m_simplify = settings.value("display/simplify", 2).toInt();

#ifdef QT_NO_OPENGL
  m_use_opengl = false;
#endif
  set_opengl(m_use_opengl);
  set_antialiasing(m_antialiasing);
  set_restrict_floor(m_restrict_floor);
  set_restrict_path(m_restrict_path);

  scene()->setBackgroundBrush(QBrush(m_background_color));
  m_map_item->set_wall_color(m_wall_color);
  m_floor_item->set_floor_color(m_floor_color);
  m_floor_item->setVisible(m_draw_floor);
  m_entity_item->setVisible(m_draw_entities);

  slot_set_display_mode(0);

  setStatusTip(
      tr("Scroll to zoom, click and drag to move and right-click for more "
         "actions"));

  connect(&m_segment_generator,
          &Valeronoi::util::SegmentGenerator::generated_segments, this,
          [=](const Valeronoi::state::DataSegments &segments) {
            qDebug() << "Calculated new Voronoi segments";
            m_measurement_item->set_data_segments(segments);
          });

  slot_map_updated();
}

void DisplayWidget::paintEvent(QPaintEvent *event) {
  (void)event;
  QGraphicsView::paintEvent(event);
}

QColor DisplayWidget::get_background_color() const {
  return m_background_color;
}

void DisplayWidget::set_background_color(QColor color) {
  QSettings settings;
  m_background_color = color;
  settings.setValue("display/backgroundColor", m_background_color);
  scene()->setBackgroundBrush(QBrush(m_background_color));
  update();
}

QColor DisplayWidget::get_wall_color() const { return m_wall_color; }

void DisplayWidget::set_wall_color(QColor color) {
  QSettings settings;
  m_wall_color = color;
  settings.setValue("display/wallColor", m_wall_color);
  m_map_item->set_wall_color(m_wall_color);
  update();
}

void DisplayWidget::set_floor(bool enabled, QColor color) {
  QSettings settings;
  m_draw_floor = enabled;
  m_floor_color = color;
  settings.setValue("display/floorColor", m_floor_color);
  settings.setValue("display/drawFloor", m_draw_floor);
  m_floor_item->set_floor_color(m_floor_color);
  m_floor_item->setVisible(m_draw_floor);
  update();
}

bool DisplayWidget::get_floor_enabled() const { return m_draw_floor; }

QColor DisplayWidget::get_floor_color() const { return m_floor_color; }

void DisplayWidget::set_entities(bool enabled) {
  QSettings settings;
  m_draw_entities = enabled;
  settings.setValue("display/drawEntities", m_draw_entities);
  m_entity_item->setVisible(m_draw_entities);
  update();
}

bool DisplayWidget::get_entities_enabled() const { return m_draw_entities; }

void DisplayWidget::set_color_map(
    const Valeronoi::util::RGBColorMap *color_map) {
  QSettings settings;
  settings.setValue("display/colorMap",
                    QString::fromStdString(color_map->name()));
  m_color_map = color_map;
  m_measurement_item->set_color_map(m_color_map);
}

qreal DisplayWidget::get_zoom_factor() const { return transform().m11(); }

void DisplayWidget::wheelEvent(QWheelEvent *event) {
  zoom_by(qPow(1.5, event->angleDelta().y() / 240.0));
}

void DisplayWidget::zoom_by(qreal factor) {
  const qreal currentZoom = get_zoom_factor();
  if ((factor < 1 && currentZoom < 0.1) || (factor > 1 && currentZoom > 100))
    return;
  scale(factor, factor);
}

void DisplayWidget::slot_map_updated() {
  m_map_item->map_updated();
  m_floor_item->map_updated();
  m_entity_item->map_updated();
  m_measurement_item->map_updated();

  m_floor_path = m_floor_item->get_floor_path();
  if (m_restrict_floor) {
    m_measurement_item->set_restrict_path(m_floor_path);
  } else {
    m_measurement_item->set_restrict_path(false);
  }
  slot_update_map_rect();
}

void DisplayWidget::slot_measurements_updated() {
  const auto &measurements = m_measurements.get_measurements();
  qDebug() << "Requesting generation of Voronoi segments";
  m_segment_generator.generate(measurements, m_display_mode, m_simplify, m_wifiFilter);
}

void DisplayWidget::slot_set_display_mode(int display_mode) {
  if (display_mode < 0 ||
      display_mode > static_cast<int>(Valeronoi::state::DISPLAY_MODE::None)) {
    return;
  }
  m_display_mode = static_cast<Valeronoi::state::DISPLAY_MODE>(display_mode);
  m_measurement_item->set_display_mode(m_display_mode);
  slot_measurements_updated();
}

void DisplayWidget::set_opengl(bool enabled) {
  QSettings settings;
  m_use_opengl = enabled;
  settings.setValue("display/useOpenGL", m_use_opengl);
#ifndef QT_NO_OPENGL
  if (m_use_opengl) {
    auto *widget = new QOpenGLWidget;
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    widget->setFormat(format);
    setViewport(widget);
  } else {
#endif
    setViewport(new QWidget);
#ifndef QT_NO_OPENGL
  }
#endif
}

bool DisplayWidget::get_opengl() const { return m_use_opengl; }

void DisplayWidget::set_antialiasing(bool enabled) {
  QSettings settings;
  m_antialiasing = enabled;
  if (m_antialiasing) {
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  } else {
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::TextAntialiasing, false);
  }
  settings.setValue("display/antialiasing", m_antialiasing);
}
bool DisplayWidget::get_antialiasing() const { return m_antialiasing; }

bool DisplayWidget::get_restrict_floor() const { return m_restrict_floor; }

void DisplayWidget::set_restrict_floor(bool enabled) {
  m_restrict_floor = enabled;
  if (m_restrict_floor) {
    m_measurement_item->set_restrict_path(m_floor_path);
  } else {
    m_measurement_item->set_restrict_path(false);
  }
  update();
}

void DisplayWidget::set_restrict_path(bool enabled) {
  m_restrict_path = enabled;
  m_measurement_item->set_restrict_points(m_restrict_path);
  update();
}

bool DisplayWidget::get_restrict_path() const { return m_restrict_path; }

void DisplayWidget::slot_update_map_rect() {
  scene()->setSceneRect(scene()->itemsBoundingRect());
}

int DisplayWidget::get_simplify() const { return m_simplify; }

void DisplayWidget::slot_set_simplify(int value) {
  int new_value = std::max(value, 1);
  if (m_simplify != new_value) {
    QSettings settings;
    m_simplify = new_value;
    settings.setValue("display/simplify", m_simplify);
    slot_measurements_updated();
  }
}

int DisplayWidget::get_wifiIdFilter() const
{
  return m_wifiFilter;
}

void DisplayWidget::slot_set_wifiIdFilter(int newWifiIdFilter)
{
  if (m_wifiFilter != newWifiIdFilter) {
    m_wifiFilter = newWifiIdFilter;
    slot_measurements_updated();
  }
}


}  // namespace Valeronoi::gui::widget
