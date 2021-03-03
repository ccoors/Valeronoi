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
                Valeronoi::state::DISPLAY_MODE display_mode, int simplify);

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
};

}  // namespace Valeronoi::util

#endif
