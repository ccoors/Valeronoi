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
#include "mdns_discovery.h"

#include <QDataStream>
#include <QNetworkInterface>

namespace Valeronoi::robot {

MdnsDiscovery::MdnsDiscovery(QObject* parent) : QObject(parent) {
  m_udpSocket = new QUdpSocket(this);
  m_udpSocket->bind(QHostAddress::AnyIPv4, 0,
                    QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

  m_queryTimer = new QTimer(this);
  connect(m_queryTimer, &QTimer::timeout, this, &MdnsDiscovery::sendQuery);

  connect(m_udpSocket, &QUdpSocket::readyRead, this,
          &MdnsDiscovery::readPendingDatagrams);
}

void MdnsDiscovery::startDiscovery() {
  sendQuery();
  m_queryTimer->start(5000);
}

void MdnsDiscovery::sendQuery() {
  QByteArray datagram;
  QDataStream out(&datagram, QIODevice::WriteOnly);
  out.setByteOrder(QDataStream::BigEndian);

  // Transaction ID
  out << (quint16)0;
  // Flags: Standard query
  out << (quint16)0x0000;
  // Questions
  out << (quint16)1;
  // Answer RRs
  out << (quint16)0;
  // Authority RRs
  out << (quint16)0;
  // Additional RRs
  out << (quint16)0;

  // Query: _valetudo._tcp.local
  static const char* service = "\x09_valetudo\x04_tcp\x05local";
  datagram.append(service, 21);
  datagram.append('\0');

  out.device()->seek(datagram.size());
  // Type: PTR
  out << (quint16)12;
  // Class: IN
  out << (quint16)1;

  m_udpSocket->writeDatagram(datagram, QHostAddress("224.0.0.251"), 5353);
}

void MdnsDiscovery::readPendingDatagrams() {
  while (m_udpSocket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(m_udpSocket->pendingDatagramSize());
    QHostAddress sender;
    m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender);

    // Very basic DNS response parsing
    if (datagram.size() < 12) continue;

    QDataStream in(&datagram, QIODevice::ReadOnly);
    in.setByteOrder(QDataStream::BigEndian);

    quint16 id, flags, questions, answers, authority, additional;
    in >> id >> flags >> questions >> answers >> authority >> additional;

    // Skip questions
    for (int i = 0; i < questions; ++i) {
      quint8 len;
      do {
        in >> len;
        if (len & 0xC0) {  // Compression
          quint8 next;
          in >> next;
          break;
        }
        if (len > 0) in.skipRawData(len);
      } while (len > 0);
      in.skipRawData(4);  // Type and Class
    }

    // Check answers for PTR records
    for (int i = 0; i < answers + authority + additional; ++i) {
      // Read name
      quint8 len;
      do {
        if (in.atEnd()) break;
        in >> len;
        if (len & 0xC0) {
          quint8 next;
          in >> next;
          break;
        }
        if (len > 0) in.skipRawData(len);
      } while (len > 0);

      quint16 type, cls;
      quint32 ttl;
      quint16 rdlength;
      in >> type >> cls >> ttl >> rdlength;

      if (type == 12) {  // PTR
        // This is a PTR record. It might contain the service instance name.
        // For Valetudo, we just care that SOMEONE responded.
        DiscoveredRobot robot;
        robot.address = sender;
        robot.name = sender.toString();
        // Try to find a TXT or SRV record in the same datagram that might have
        // a better name Valetudo typically puts the robot name in a TXT record
        // or the service instance name. For now, we'll just use the IP or
        // hostname if we can find it.
        emit robotDiscovered(robot);
      } else if (type == 33) {  // SRV
        // SRV record can give us the port and target hostname
        quint16 priority, weight, port;
        in >> priority >> weight >> port;
        // Target name follows
        // We'll just emit it if we haven't already
        DiscoveredRobot robot;
        robot.address = sender;
        robot.port = port;
        robot.name = sender.toString();
        emit robotDiscovered(robot);
      } else {
        in.skipRawData(rdlength);
      }
    }
  }
}

}  // namespace Valeronoi::robot
