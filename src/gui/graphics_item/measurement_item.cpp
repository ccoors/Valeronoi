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
#include "measurement_item.h"

#include <QPen>
#include <algorithm>

constexpr const int SCALE_FONT_SIZE{15};
constexpr const int SCALE_HISTOGRAM_HEIGHT{70};
constexpr const int SCALE_BAR_HEIGHT{30};
constexpr const int SCALE_WIDTH{200};
constexpr const int SCALE_MARGIN{5};
constexpr const int PATH_DISTANCE{35};
constexpr const QSize SCALE_SIZE{
    SCALE_WIDTH, SCALE_BAR_HEIGHT + SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT};

namespace Valeronoi::gui::graphics_item {
MeasurementItem::MeasurementItem(const Valeronoi::state::RobotMap &robot_map,
                                 QGraphicsItem *parent)
    : MapBasedItem(robot_map, parent), m_font("Source Code Pro") {
  m_font.setPixelSize(SCALE_FONT_SIZE);
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
  return {static_cast<int>(255 * color[0]), static_cast<int>(255 * color[1]),
          static_cast<int>(255 * color[2])};
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
  m_histogram.clear();
  m_histogram_max = 0;
  for (const auto &s : m_data_segments) {
    m_min = std::min(m_min, s.value);
    m_max = std::max(m_max, s.value);
    const auto int_value = static_cast<int>(s.value);
    m_histogram[int_value] += 1;
    m_histogram_max = std::max(m_histogram_max, m_histogram[int_value]);
    m_points_path.addRect(s.x - PATH_DISTANCE, s.y - PATH_DISTANCE,
                          2 * PATH_DISTANCE, 2 * PATH_DISTANCE);
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
    painter.drawRect(0, 0, SCALE_WIDTH + 2 * SCALE_MARGIN,
                     SCALE_SIZE.height() + 2 * SCALE_MARGIN);

    if (m_histogram_max > 0) {
      painter.setPen(Qt::transparent);
      const auto int_max = static_cast<int>(m_max);
      const auto int_min = static_cast<int>(m_min);
      const auto bar_width =
          static_cast<double>(SCALE_WIDTH) / (int_max - int_min + 1);
      std::for_each(
          m_histogram.begin(), m_histogram.end(), [&](const auto values) {
            const auto bin = values.first;
            const auto count = values.second;
            const auto height = SCALE_HISTOGRAM_HEIGHT *
                                static_cast<double>(count) / m_histogram_max;
            const auto x = SCALE_MARGIN + bar_width * (bin - int_min);
            auto bin_rect =
                QRectF(x, SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT - height,
                       bar_width, height);
            painter.setBrush(get_color(bin));
            painter.drawRect(bin_rect);
          });
      painter.setBrush(Qt::transparent);
      painter.setPen(QPen(Qt::white, 1));
      painter.drawRect(SCALE_MARGIN, SCALE_MARGIN, SCALE_WIDTH,
                       SCALE_HISTOGRAM_HEIGHT);
    }

    painter.setPen(Qt::transparent);

    for (int x = 0; x < SCALE_WIDTH; x++) {
      painter.setBrush(color_value(static_cast<double>(x) / SCALE_WIDTH));
      painter.drawRect(x + SCALE_MARGIN,
                       2 * SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT, 1,
                       SCALE_BAR_HEIGHT / 3);
    }
    painter.setBrush(Qt::transparent);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRect(SCALE_MARGIN, 2 * SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT,
                     SCALE_WIDTH, SCALE_BAR_HEIGHT / 3);
    painter.drawText(
        SCALE_MARGIN,
        2 * SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT + SCALE_BAR_HEIGHT / 3,
        SCALE_WIDTH / 2, (SCALE_BAR_HEIGHT / 3) * 2, Qt::AlignTop,
        QString::number(m_min, 'f', 1).append(" dBm"));
    painter.drawText(
        SCALE_MARGIN + SCALE_WIDTH / 2,
        2 * SCALE_MARGIN + SCALE_HISTOGRAM_HEIGHT + SCALE_BAR_HEIGHT / 3,
        SCALE_WIDTH / 2, (SCALE_BAR_HEIGHT / 3) * 2,
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
  if (rect.width() < SCALE_WIDTH + SCALE_MARGIN) {
    rect += QMarginsF(0, 0, SCALE_WIDTH + SCALE_MARGIN, 0);
  }
  return rect + QMarginsF(0, 0, 0, SCALE_SIZE.height() + 4 * SCALE_MARGIN);
}

}  // namespace Valeronoi::gui::graphics_item
