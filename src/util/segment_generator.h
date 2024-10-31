/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2024 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_UTIL_SEGMENT_GENERATOR_H
#define VALERONOI_UTIL_SEGMENT_GENERATOR_H

#include <QMutex>
#include <QObject>
#include <QSize>
#include <QThread>
#include <QWaitCondition>

#include "../state/state.h"

namespace Valeronoi::util {

class SegmentGenerator : public QThread {
  Q_OBJECT
 public:
  ~SegmentGenerator() override;

  void generate(const Valeronoi::state::RawMeasurements &measurements,
                Valeronoi::state::DISPLAY_MODE display_mode, int simplify,
                int wifi_id_filter = -1);

 signals:
  void generated_segments(const Valeronoi::state::DataSegments &segments);

 protected:
  void run() override;

 private:
  static void generate_voronoi(
      const Valeronoi::state::RawMeasurements &measurements,
      Valeronoi::state::DataSegments &segments);

  bool m_abort{false}, m_restart{false};
  QMutex m_mutex;
  QWaitCondition m_condition;

  Valeronoi::state::RawMeasurements m_measurements{};
  Valeronoi::state::DISPLAY_MODE m_display_mode{};
  int m_simplify;
  int m_wifi_id_filter;
};

}  // namespace Valeronoi::util

#endif
