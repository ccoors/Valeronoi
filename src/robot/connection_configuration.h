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
#ifndef VALERONOI_ROBOT_CONNECTION_INFORMATION_H
#define VALERONOI_ROBOT_CONNECTION_INFORMATION_H

#include <QNetworkRequest>
#include <QString>
#include <QUrl>

namespace Valeronoi::robot {

struct ConnectionConfiguration {
  QUrl m_url;
  bool m_auth;
  QString m_username, m_password;

  void read_settings();

  [[nodiscard]] bool is_valid() const;

  ConnectionConfiguration& operator=(const ConnectionConfiguration& other);

  void prepare_request(QNetworkRequest& r) const;
};

}  // namespace Valeronoi::robot

#endif
