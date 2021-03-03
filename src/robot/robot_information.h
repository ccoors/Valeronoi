#ifndef VALETUDO_ROBOT_ROBOT_INFORMATION_H
#define VALETUDO_ROBOT_ROBOT_INFORMATION_H

#include <QString>
#include <QStringList>

namespace Valeronoi::robot {

struct RobotInformation {
  QString m_valetudo_version;
  QString m_manufacturer, m_model_name, m_implementation;
  QStringList m_capabilities;
};

}  // namespace Valeronoi::robot

#endif
