/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_ROBOT_MDNS_DISCOVERY_H
#define VALERONOI_ROBOT_MDNS_DISCOVERY_H

#include <QHostAddress>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

namespace Valeronoi::robot {

struct DiscoveredRobot {
  QString name;
  QString host;
  quint16 port;
  QHostAddress address;

  QString url() const {
    return QString("http://%1")
        .arg(address.isNull() ? host : address.toString());
  }
};

class MdnsDiscovery : public QObject {
  Q_OBJECT
 public:
  explicit MdnsDiscovery(QObject* parent = nullptr);

  void startDiscovery();

 signals:
  void robotDiscovered(const DiscoveredRobot& robot);

 private slots:
  void readPendingDatagrams();
  void sendQuery();

 private:
  QUdpSocket* m_udpSocket;
  QTimer* m_queryTimer;
};

}  // namespace Valeronoi::robot

#endif  // VALERONOI_ROBOT_MDNS_DISCOVERY_H
