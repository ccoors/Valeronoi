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
#ifndef VALETUDO_ROBOT_WIFI_INFORMATION_H
#define VALETUDO_ROBOT_WIFI_INFORMATION_H

#include <QJsonObject>
#include <QSharedDataPointer>

namespace Valeronoi::robot {

class WifiInformationData;

class WifiInformation {
 public:
  WifiInformation();
  explicit WifiInformation(const QJsonObject &jsonObj);
  explicit WifiInformation(double signal);
  WifiInformation(double signal, const QString &ssid, const QString &bssid);

  WifiInformation(const WifiInformation &);
  WifiInformation &operator=(const WifiInformation &);
  ~WifiInformation();

  void set_ssid(const QString &ssid);
  void set_bssid(const QString &bssid);
  void set_signal(double signal);

  [[nodiscard]] QString ssid() const;
  [[nodiscard]] QString bssid() const;
  [[nodiscard]] double signal() const;

  [[nodiscard]] QJsonObject get_json() const;
  void set_json(QJsonObject jsonObj);

  [[nodiscard]] bool has_valid_signal() const;

 private:
  QSharedDataPointer<WifiInformationData> m_data;
};

}  // namespace Valeronoi::robot
#endif  // VALETUDO_ROBOT_WIFI_INFORMATION_H
