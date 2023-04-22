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
#ifndef VALETUDO_ROBOT_ROBOT_INFORMATION_H
#define VALETUDO_ROBOT_ROBOT_INFORMATION_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace Valeronoi::robot {

struct RobotInformation {
  QString m_valetudo_version;
  QString m_manufacturer, m_model_name, m_implementation;
  QStringList m_capabilities;
  QJsonArray m_attributes;

  [[nodiscard]] QJsonObject get_attribute(const QString &class_name,
                                          const QString &type = "",
                                          const QString &sub_type = "") const;
};

}  // namespace Valeronoi::robot

#endif
