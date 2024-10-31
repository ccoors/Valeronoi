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

#include "wifi_collection.h"

#include "qjsonarray.h"

namespace Valeronoi::state {

wifi_collection::wifi_collection() { clear(); }

void wifi_collection::clear() { m_known_wifis.clear(); }

int wifi_collection::get_or_create_wifi_id(
    const Valeronoi::robot::WifiInformation& wifi_info) {
  int nRet = get_wifi_id(wifi_info.bssid());

  if (nRet < 0) {
    nRet = add_wifi(wifi_info);
  }

  return nRet;
}

int wifi_collection::add_wifi(
    const Valeronoi::robot::WifiInformation& wifi_info) {
  m_known_wifis.append(wifi_info);

  emit signal_wifi_list_updated();
  emit signal_new_wifi_added(wifi_info);

  return m_known_wifis.size() - 1;
}

QVector<Valeronoi::robot::WifiInformation> wifi_collection::get_known_wifis()
    const {
  return m_known_wifis;
}

QJsonArray wifi_collection::get_json() const {
  QJsonArray ret;

  for (const auto& m_known_wifi : m_known_wifis) {
    ret.append(m_known_wifi.get_json());
  }

  return ret;
}

void wifi_collection::set_json(const QJsonArray& json) {
  clear();

  for (const auto& wifi_access_point_value : json) {
    if (!wifi_access_point_value.isObject()) {
      // Wrong format.
      return;
    }
    QJsonObject wifi_access_point = wifi_access_point_value.toObject();
    m_known_wifis.append(Valeronoi::robot::WifiInformation(wifi_access_point));
  }

  emit signal_wifi_list_updated();
}

int wifi_collection::get_wifi_id(const QString& bssid) const {
  for (int index = 0; m_known_wifis.size() > index; ++index) {
    if (m_known_wifis.at(index).bssid() == bssid) {
      return index;
    }
  }

  return -1;
}

}  // namespace Valeronoi::state
