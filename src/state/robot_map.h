#ifndef VALERONOI_STATE_ROBOT_MAP_H
#define VALERONOI_STATE_ROBOT_MAP_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include "state.h"

namespace Valeronoi::state {

class RobotMap : public QObject {
  Q_OBJECT
 public:
  explicit RobotMap(QObject *parent = nullptr);

  void update_map_json(const QString &json);

  void update_map_json(const QJsonObject &json);

  void reset();

  [[nodiscard]] bool is_valid() const;

  [[nodiscard]] QString error_msg() const;

  [[nodiscard]] const Map &get_map() const;

  [[nodiscard]] const QJsonObject &get_map_json() const;

 signals:
  void signal_map_updated();

 private:
  void generate_map();

  QJsonObject m_map_json{};
  bool m_valid{false};
  QString m_error;
  Map m_map;
};
}  // namespace Valeronoi::state

#endif
