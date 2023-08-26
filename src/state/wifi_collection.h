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
#ifndef VALERONOI_STATE_WIFI_COLLECTION_H
#define VALERONOI_STATE_WIFI_COLLECTION_H

#include <QJsonObject>
#include <QObject>
#include <QVector>

#include "../robot/wifi_information.h"

namespace Valeronoi::state {

class wifi_collection : public QObject {
  Q_OBJECT
 public:
  wifi_collection();

  void clear();

  int get_or_create_wifi_id(const Valeronoi::robot::WifiInformation& wifi_info);

  [[nodiscard]] int get_wifi_id(const QString& bssid) const;

  int add_wifi(const Valeronoi::robot::WifiInformation& wifi_info);

  [[nodiscard]] QVector<Valeronoi::robot::WifiInformation> get_known_wifis()
      const;

  [[nodiscard]] QJsonArray get_json() const;

  void set_json(const QJsonArray& json);

 signals:
  void signal_wifi_list_updated();

  void signal_new_wifi_added(Valeronoi::robot::WifiInformation wifi_info);

 private:
  QVector<Valeronoi::robot::WifiInformation> m_known_wifis;
};

}  // namespace Valeronoi::state

#endif  // VALERONOI_STATE_WIFI_COLLECTION_H
