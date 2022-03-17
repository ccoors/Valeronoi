/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2022 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_GUI_GRAPHICS_ITEM_MEASUREMENT_ITEM_H
#define VALERONOI_GUI_GRAPHICS_ITEM_MEASUREMENT_ITEM_H

#include <QFont>
#include <QPicture>
#include <unordered_map>

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
  std::unordered_map<int, int> m_histogram;
  int m_histogram_max{0};

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
