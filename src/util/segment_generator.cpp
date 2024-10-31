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
#include "segment_generator.h"

#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_policies_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_traits_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Voronoi_diagram_2.h>

#include <utility>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> DT;
typedef CGAL::Delaunay_triangulation_adaptation_traits_2<DT> AT;
typedef CGAL::Delaunay_triangulation_caching_degeneracy_removal_policy_2<DT> AP;
typedef CGAL::Voronoi_diagram_2<DT, AT, AP> VD;

typedef AT::Site_2 Site_2;
typedef AT::Point_2 Point_2;
typedef VD::Locate_result Locate_result;
typedef VD::Face_handle Face_handle;
typedef VD::Halfedge_handle Halfedge_handle;
typedef VD::Ccb_halfedge_circulator Ccb_halfedge_circulator;

#if CGAL_VERSION_NR < CGAL_VERSION_NUMBER(6, 0, 0)
#define GET_IF boost::get<Face_handle>
#else
#define GET_IF std::get_if<Face_handle>
#endif

namespace Valeronoi::util {

SegmentGenerator::~SegmentGenerator() {
  m_mutex.lock();
  m_abort = true;
  m_condition.wakeOne();
  m_mutex.unlock();

  wait();
}

void SegmentGenerator::generate(
    const Valeronoi::state::RawMeasurements &measurements,
    Valeronoi::state::DISPLAY_MODE display_mode, int simplify,
    int wifi_id_filter) {
  QMutexLocker locker(&m_mutex);

  m_measurements = measurements;
  m_display_mode = display_mode;
  m_simplify = simplify;
  m_wifi_id_filter = wifi_id_filter;

  if (!isRunning()) {
    start(LowPriority);
  } else {
    m_restart = true;
    m_condition.wakeOne();
  }
}

void SegmentGenerator::run() {
  while (true) {
    m_mutex.lock();
    auto measurements = m_measurements;
    const auto display_mode = m_display_mode;
    const auto simplify = m_simplify;
    const auto wifi_id_filter = m_wifi_id_filter;
    m_mutex.unlock();
    if (m_abort) {
      return;
    }

    Valeronoi::state::RawMeasurements processed_measurements;
    if (simplify > 1 || wifi_id_filter != -1) {
      for (auto &m : measurements) {
        bool saveValue = true;

        if (wifi_id_filter != -1 && m.wifi_id != wifi_id_filter) {
          continue;  // Skip and probe next one
        }

        if (simplify > 1) {
          m.x = (m.x / simplify) * simplify;
          m.y = (m.y / simplify) * simplify;

          for (auto &sm : processed_measurements) {
            if (sm.x == m.x && sm.y == m.y) {
              // I'd like to use std::transform with std::back_inserter instead,
              // but that doesn't work for doubles
              for (const auto d : m.data) {
                sm.data.push_back(d);
              }
              saveValue = false;
            }
          }
        }

        if (saveValue) {
          processed_measurements.push_back(m);
        }
      }
      // Update averages
      for (auto &sm : processed_measurements) {
        double avg = 0;
        for (const auto &m : sm.data) {
          avg += m;
        }
        sm.average = avg / sm.data.size();
      }
    } else {
      processed_measurements = std::move(measurements);
    }

    Valeronoi::state::DataSegments segments;

    switch (display_mode) {
      case state::DISPLAY_MODE::Voronoi:
        generate_voronoi(processed_measurements, segments);
        break;
      case state::DISPLAY_MODE::DataPoints:
        for (const auto &m : processed_measurements) {
          Valeronoi::state::DataSegment s;
          s.x = m.x;
          s.y = m.y;
          s.value = m.average;
          segments.push_back(s);
        }
        break;
      case state::DISPLAY_MODE::None:
        break;
    }

    emit generated_segments(segments);

    m_mutex.lock();
    if (!m_restart) {
      m_condition.wait(&m_mutex);
    }
    m_restart = false;
    m_mutex.unlock();
  }
}

void SegmentGenerator::generate_voronoi(
    const Valeronoi::state::RawMeasurements &measurements,
    Valeronoi::state::DataSegments &segments) {
  if (measurements.size() < 2) {
    return;
  }
  VD vd;
  int x_max{0}, y_max{0};
  for (const auto &m : measurements) {
    Site_2 t(m.x, m.y);
    vd.insert(t);
    x_max = std::max(x_max, m.x);
    y_max = std::max(y_max, m.y);
  }
  const int x_center{x_max / 2};
  const int y_center{y_max / 2};

  // Insert dummy points to prevent infinite points in our vertices.
  // You could also do a ton of math magic to calculate the infinite points
  // in drawing space, but this is just easier and less likely to contain bugs.
  for (int y = -1; y < 2; y++) {
    for (int x = -1; x < 2; x++) {
      if (x || y) {  // No center point
        Site_2 t(x_center + x * (x_center * 10),
                 y_center + y * (y_center * 10));
        vd.insert(t);
      }
    }
  }

  if (!vd.is_valid()) {
    return;
  }
  for (const auto &m : measurements) {
    Point_2 p(m.x, m.y);
    auto result = vd.locate(p);
    if (auto *v = GET_IF(&result)) {
      Valeronoi::state::DataSegment s;
      s.x = m.x;
      s.y = m.y;
      Ccb_halfedge_circulator ec_start = (*v)->ccb();
      Ccb_halfedge_circulator ec = ec_start;
      do {
        if (ec->has_source()) {
          const auto point = ec->source()->point();
          s.polygon << QPoint(point.x(), point.y());
        }
      } while (++ec != ec_start);
      s.value = m.average;
      segments.push_back(s);
    } else {
      // Point is not on a face
    }
  }
}

}  // namespace Valeronoi::util
