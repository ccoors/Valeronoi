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
#include <QMap>
#include <QObject>
#include <QSet>
#include <QTimer>

class QSocketNotifier;

namespace Valeronoi::robot {

struct DiscoveredRobot {
  QString name;
  QString id;
  QString model;
  QString manufacturer;
  QString version;
  QString host;
  quint16 port;
  QHostAddress address;

  [[nodiscard]] QString url() const {
    if (port != 80) {
      return QString("http://%1:%2")
          .arg(address.isNull() ? host : address.toString(),
               QString::number(port));
    }
    return QString("http://%1")
        .arg(address.isNull() ? host : address.toString());
  }
};

class MdnsDiscovery : public QObject {
  Q_OBJECT
 public:
  // Accessible to the mdns callback in the .cpp translation unit
  struct PendingService {
    QString host;
    quint16 port{0};
    QHostAddress address;
    QHostAddress senderAddr;
    QString friendlyName;
    QString id;
    QString model;
    QString manufacturer;
    QString version;
    bool hasSrv{false};
  };

  explicit MdnsDiscovery(QObject* parent = nullptr);
  ~MdnsDiscovery() override;

  void startDiscovery();

 signals:
  void robotDiscovered(const DiscoveredRobot& robot);

 private slots:
  void onSocketReady();
  void sendQuery() const;

 private:
  int m_sock{-1};
  QSocketNotifier* m_notifier{nullptr};
  QTimer* m_queryTimer;
  QMap<QString, PendingService> m_pending;
  QSet<QString> m_emitted;
};

}  // namespace Valeronoi::robot

#endif  // VALERONOI_ROBOT_MDNS_DISCOVERY_H
