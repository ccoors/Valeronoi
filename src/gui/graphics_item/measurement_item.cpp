#include "measurement_item.h"

#include <QPen>

constexpr const QSize SCALE_SIZE{200, 30};
constexpr const int SCALE_MARGIN{5};

namespace Valeronoi::gui::graphics_item {
MeasurementItem::MeasurementItem(const Valeronoi::state::RobotMap &robot_map,
                                 QGraphicsItem *parent)
    : MapBasedItem(robot_map, parent), m_font("Source Code Pro") {
  m_font.setPixelSize(SCALE_SIZE.height() / 2);
}

void MeasurementItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  (void)option;
  (void)widget;
  (void)painter;
  painter->setPen(Qt::transparent);
  if (m_display_mode == Valeronoi::state::DISPLAY_MODE::None ||
      m_color_map == nullptr || m_min >= m_max) {
    return;
  }
  if (m_robot_map.is_valid()) {
    if (m_display_mode == Valeronoi::state::DISPLAY_MODE::Voronoi) {
      // Rendering Voronoi segments with antialiasing leads to artifacts
      painter->setRenderHints(QPainter::Antialiasing, false);
      painter->setClipRect(MapBasedItem::boundingRect(), Qt::ReplaceClip);
      if (m_restrict_path) {
        painter->setClipPath(m_path, Qt::IntersectClip);
      }
      if (m_restrict_points) {
        painter->setClipPath(m_points_path, Qt::IntersectClip);
      }

      for (const auto &p : m_data_segments) {
        painter->setBrush(p.color);
        painter->drawPolygon(p.polygon);
      }
    } else if (m_display_mode == Valeronoi::state::DISPLAY_MODE::DataPoints) {
      for (const auto &p : m_data_segments) {
        painter->setBrush(p.color);
        painter->drawEllipse(p.x - 2, p.y - 2, 4, 4);
      }
    }

    painter->setClipRect(boundingRect(), Qt::ReplaceClip);
    const auto &map = m_robot_map.get_map();
    const auto base_y = map.size_y + SCALE_MARGIN;
    if (!m_legend.isNull()) {
      painter->drawPicture(QPoint(SCALE_MARGIN, base_y), m_legend);
    }
  }
}

void MeasurementItem::set_display_mode(
    Valeronoi::state::DISPLAY_MODE display_mode) {
  m_display_mode = display_mode;
  calculate_colors();
}

void MeasurementItem::set_color_map(
    const Valeronoi::util::RGBColorMap *color_map) {
  m_color_map = color_map;
  calculate_colors();
}

QColor MeasurementItem::color_value(double normalized_value) const {
  auto color = m_color_map->get_color(normalized_value);
  return QColor(static_cast<int>(255 * color[0]),
                static_cast<int>(255 * color[1]),
                static_cast<int>(255 * color[2]));
}

QColor MeasurementItem::get_color(double value) const {
  if (m_min == m_max || m_max - m_min == 0 || m_min > m_max || !m_color_map) {
    return Qt::black;
  }
  double normalized = (value - m_min) / (m_max - m_min);
  return color_value(normalized);
}

void MeasurementItem::set_data_segments(const state::DataSegments &segments) {
  m_points_path.clear();
  m_points_path.setFillRule(Qt::WindingFill);
  m_data_segments = segments;
  m_min = 0.0;
  m_max = -100.0;
  for (const auto &s : m_data_segments) {
    m_min = std::min(m_min, s.value);
    m_max = std::max(m_max, s.value);
    m_points_path.addRect(s.x - 25, s.y - 25, 50, 50);
  }
  calculate_colors();
}

void MeasurementItem::calculate_colors() {
  for (auto &s : m_data_segments) {
    s.color = get_color(s.value);
  }
  if (m_color_map && m_max > m_min && m_robot_map.is_valid()) {
    QPainter painter;
    painter.begin(&m_legend);
    painter.setFont(m_font);

    painter.setBrush(Qt::black);
    painter.setPen(Qt::transparent);
    painter.drawRect(0, 0, SCALE_SIZE.width() + 2 * SCALE_MARGIN,
                     SCALE_SIZE.height() + 2 * SCALE_MARGIN);
    for (int x = 0; x < SCALE_SIZE.width(); x++) {
      painter.setBrush(
          color_value(static_cast<double>(x) / SCALE_SIZE.width()));
      painter.drawRect(x + SCALE_MARGIN, SCALE_MARGIN, 1,
                       SCALE_SIZE.height() / 3);
    }
    painter.setBrush(Qt::transparent);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRect(SCALE_MARGIN, SCALE_MARGIN, SCALE_SIZE.width(),
                     SCALE_SIZE.height() / 3);
    painter.drawText(SCALE_MARGIN, SCALE_MARGIN + SCALE_SIZE.height() / 3,
                     SCALE_SIZE.width() / 2, (SCALE_SIZE.height() / 3) * 2,
                     Qt::AlignTop,
                     QString::number(m_min, 'f', 1).append(" dBm"));
    painter.drawText(SCALE_MARGIN + SCALE_SIZE.width() / 2,
                     SCALE_MARGIN + SCALE_SIZE.height() / 3,
                     SCALE_SIZE.width() / 2, (SCALE_SIZE.height() / 3) * 2,
                     Qt::AlignTop | Qt::AlignRight,
                     QString::number(m_max, 'f', 1).append(" dBm"));
    painter.end();
  }
  update();
}

void MeasurementItem::set_restrict_path(bool enabled) {
  m_restrict_path = enabled;
  update();
}

void MeasurementItem::set_restrict_path(const QPainterPath &path) {
  m_path = path;
  m_path.setFillRule(Qt::WindingFill);
  m_restrict_path = true;
  update();
}

void MeasurementItem::set_restrict_points(bool enabled) {
  m_restrict_points = enabled;
  update();
}

QRectF MeasurementItem::boundingRect() const {
  auto rect = MapBasedItem::boundingRect();
  if (!m_robot_map.is_valid()) {
    return rect;
  }
  if (rect.width() < SCALE_SIZE.width() + SCALE_MARGIN) {
    rect += QMarginsF(0, 0, SCALE_SIZE.width() + SCALE_MARGIN, 0);
  }
  return rect + QMarginsF(0, 0, 0, SCALE_SIZE.height() + 4 * SCALE_MARGIN);
}

}  // namespace Valeronoi::gui::graphics_item
