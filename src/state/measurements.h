#ifndef VALERONOI_STATE_MEASUREMENTS_H
#define VALERONOI_STATE_MEASUREMENTS_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <vector>

#include "robot_map.h"
#include "state.h"

namespace Valeronoi::state {

class Measurements : public QObject {
  Q_OBJECT
 public:
  void reset();

  void set_json(const QJsonArray &json);

  void set_map(const RobotMap &map);

  [[nodiscard]] const RawMeasurements &get_measurements() const;

  [[nodiscard]] QJsonArray get_json() const;

  [[nodiscard]] MeasurementStatistics get_statistics() const;

 signals:
  void signal_measurements_updated();

 public slots:
  void slot_add_measurement(double data);

 private:
  void add_measurement(int x, int y, double value);

  const RobotMap *m_map{nullptr};

  //  QJsonArray m_data;
  std::vector<Measurement> m_data;
};

}  // namespace Valeronoi::state

#endif
