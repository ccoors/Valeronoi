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
#include "connection_configuration.h"

#include <QSettings>

#include "config.h"

namespace Valeronoi::robot {

void ConnectionConfiguration::read_settings() {
  QSettings settings;
  m_url = settings.value("robot/url", "").toUrl();
  m_auth = settings.value("robot/auth/enabled", false).toBool();
  m_username = settings.value("robot/auth/username", "").toString();
  m_password = settings.value("robot/auth/password", "").toString();
}

ConnectionConfiguration &ConnectionConfiguration::operator=(
    const ConnectionConfiguration &other) {
  if (this != &other) {
    m_url = other.m_url;
    m_auth = other.m_auth;
    m_username = other.m_username;
    m_password = other.m_password;
  }
  return *this;
}

bool ConnectionConfiguration::is_valid() const {
  return !m_url.isEmpty() && m_url.isValid();
}

void ConnectionConfiguration::prepare_request(QNetworkRequest &r) const {
  r.setMaximumRedirectsAllowed(10);  // Should not be needed anyway
  r.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                 QNetworkRequest::AlwaysNetwork);
  r.setTransferTimeout(
      5000);  // TODO configurable? // Not availabe before Qt 5.15
  r.setHeader(QNetworkRequest::UserAgentHeader, "Valeronoi/" VALERONOI_VERSION);
  if (m_auth) {
    QString user_pw = m_username + ":" + m_password;
    QByteArray data = user_pw.toLocal8Bit().toBase64();
    QString header_data = "Basic " + data;
    r.setRawHeader("Authorization", header_data.toLocal8Bit());
  }
}

}  // namespace Valeronoi::robot
