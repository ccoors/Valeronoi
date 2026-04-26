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

#include <netinet/in.h>
#include <sys/socket.h>

#include <QDebug>
#include <QNetworkInterface>
#include <QSocketNotifier>

#include "mdns.h"

namespace Valeronoi::robot {
namespace {
constexpr char VALETUDO_SERVICE[] = "_valetudo._tcp.local.";
constexpr size_t VALETUDO_SERVICE_LEN = sizeof(VALETUDO_SERVICE) - 1;
const auto VALETUDO_SERVICE_SUFFIX = QStringLiteral("._valetudo._tcp.local");
constexpr size_t RECV_BUFFER_SIZE = 2048;

struct RecvContext {
  QMap<QString, MdnsDiscovery::PendingService>* pending;
};

QString extractName(const void* data, const size_t size,
                    const size_t name_offset) {
  char buf[256];
  size_t offset = name_offset;
  auto [str, length] =
      mdns_string_extract(data, size, &offset, buf, sizeof(buf));
  QString name = QString::fromLatin1(str, static_cast<int>(length));
  if (name.endsWith(QLatin1Char('.'))) name.chop(1);
  return name;
}

int mdns_record_callback(int /*sock*/, const struct sockaddr* from,
                         size_t /*addrlen*/, mdns_entry_type_t /*entry*/,
                         uint16_t /*query_id*/, uint16_t rtype,
                         uint16_t /*rclass*/, uint32_t /*ttl*/,
                         const void* data, size_t size, size_t name_offset,
                         size_t /*name_length*/, size_t record_offset,
                         size_t record_length, void* user_data) {
  const auto* ctx = static_cast<RecvContext*>(user_data);
  const QString nameStr = extractName(data, size, name_offset);

  if (rtype == MDNS_RECORDTYPE_PTR) {
    // Only track PTR records for our service type
    if (nameStr != QStringLiteral("_valetudo._tcp.local")) return 0;
    char ptrBuf[256];
    auto [str, length] = mdns_record_parse_ptr(
        data, size, record_offset, record_length, ptrBuf, sizeof(ptrBuf));
    if (length == 0) return 0;
    QString instanceName = QString::fromLatin1(str, static_cast<int>(length));
    if (instanceName.endsWith(QLatin1Char('.'))) instanceName.chop(1);
    if (!ctx->pending->contains(instanceName)) {
      qDebug() << "mDNS: found Valetudo instance:" << instanceName;
      ctx->pending->insert(instanceName, MdnsDiscovery::PendingService{});
    }

  } else if (rtype == MDNS_RECORDTYPE_SRV) {
    if (!ctx->pending->contains(nameStr)) return 0;
    char srvBuf[256];
    mdns_record_srv_t srv = mdns_record_parse_srv(
        data, size, record_offset, record_length, srvBuf, sizeof(srvBuf));
    auto& svc = (*ctx->pending)[nameStr];
    svc.host =
        QString::fromLatin1(srv.name.str, static_cast<int>(srv.name.length));
    if (svc.host.endsWith(QLatin1Char('.'))) svc.host.chop(1);
    svc.port = srv.port;
    svc.hasSrv = true;
    if (from->sa_family == AF_INET) {
      const auto* addr4 = reinterpret_cast<const sockaddr_in*>(from);
      svc.senderAddr = QHostAddress(ntohl(addr4->sin_addr.s_addr));
    }
    qDebug() << "mDNS: SRV for" << nameStr << "→" << svc.host << "port"
             << svc.port;

  } else if (rtype == MDNS_RECORDTYPE_A) {
    sockaddr_in addr{};
    mdns_record_parse_a(data, size, record_offset, record_length, &addr);
    if (!addr.sin_addr.s_addr) return 0;
    const QHostAddress qAddr(ntohl(addr.sin_addr.s_addr));
    for (auto& svc : *ctx->pending) {
      if (svc.host == nameStr) {
        qDebug() << "mDNS: A record" << nameStr << "→" << qAddr.toString();
        svc.address = qAddr;
      }
    }

  } else if (rtype == MDNS_RECORDTYPE_TXT) {
    if (!ctx->pending->contains(nameStr)) return 0;
    mdns_record_txt_t txtRecords[16];
    size_t count = mdns_record_parse_txt(data, size, record_offset,
                                         record_length, txtRecords, 16);
    auto& svc = (*ctx->pending)[nameStr];
    for (size_t i = 0; i < count; ++i) {
      auto& rec = txtRecords[i];
      if (rec.value.length == 0) continue;
      QString val = QString::fromLatin1(rec.value.str,
                                        static_cast<int>(rec.value.length));
      const size_t kl = rec.key.length;
      const char* ks = rec.key.str;
      if (kl == 4 && memcmp(ks, "name", 4) == 0)
        svc.friendlyName = val;
      else if (kl == 2 && memcmp(ks, "id", 2) == 0)
        svc.id = val;
      else if (kl == 5 && memcmp(ks, "model", 5) == 0)
        svc.model = val;
      else if (kl == 12 && memcmp(ks, "manufacturer", 12) == 0)
        svc.manufacturer = val;
      else if (kl == 7 && memcmp(ks, "version", 7) == 0)
        svc.version = val;
    }
  }

  return 0;
}

}  // namespace

MdnsDiscovery::MdnsDiscovery(QObject* parent) : QObject(parent) {
  sockaddr_in saddr{};
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  // Bind to port 5353 so multicast responses from bonjour-service reach us.
  // An ephemeral port would cause responses sent to 224.0.0.251:5353 to be
  // delivered only to port-5353 sockets, never to the ephemeral-port socket.
  saddr.sin_port = htons(MDNS_PORT);

  m_sock = mdns_socket_open_ipv4(&saddr);
  if (m_sock >= 0) {
    // mdns.h joins the multicast group on the default interface only.
    // Join on every active interface, so we receive responses regardless of
    // which physical link the robot is on — same as avahi-daemon does.
    for (const auto& iface : QNetworkInterface::allInterfaces()) {
      const auto flags = iface.flags();
      if (!(flags & QNetworkInterface::IsUp) ||
          !(flags & QNetworkInterface::CanMulticast) ||
          (flags & QNetworkInterface::IsLoopBack))
        continue;
      for (const auto& entry : iface.addressEntries()) {
        if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
        ip_mreq req{};
        req.imr_multiaddr.s_addr = htonl(0xE00000FBU);  // 224.0.0.251
        req.imr_interface.s_addr = htonl(entry.ip().toIPv4Address());
        if (setsockopt(m_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       reinterpret_cast<const char*>(&req), sizeof(req)) == 0) {
          qDebug() << "mDNS: joined multicast on" << iface.name()
                   << entry.ip().toString();
        }
      }
    }
    qDebug() << "mDNS: socket ready on port 5353";
    m_notifier = new QSocketNotifier(m_sock, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this,
            &MdnsDiscovery::onSocketReady);
  } else {
    qWarning() << "mDNS: failed to open socket on port 5353";
  }

  m_queryTimer = new QTimer(this);
  connect(m_queryTimer, &QTimer::timeout, this, &MdnsDiscovery::sendQuery);
}

MdnsDiscovery::~MdnsDiscovery() {
  if (m_sock >= 0) mdns_socket_close(m_sock);
}

void MdnsDiscovery::startDiscovery() {
  if (m_sock < 0) return;
  sendQuery();
  m_queryTimer->start(5000);
}

void MdnsDiscovery::sendQuery() const {
  if (m_sock < 0) return;
  // Send the query on every multicast-capable interface so the robot receives
  // it even when it is not on the default-route interface.
  for (const auto& iface : QNetworkInterface::allInterfaces()) {
    if (const auto flags = iface.flags();
        !(flags & QNetworkInterface::IsUp) ||
        !(flags & QNetworkInterface::CanMulticast) ||
        (flags & QNetworkInterface::IsLoopBack))
      continue;
    for (const auto& entry : iface.addressEntries()) {
      if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
      const quint32 ifaddr = htonl(entry.ip().toIPv4Address());
      setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_IF,
                 reinterpret_cast<const char*>(&ifaddr), sizeof(ifaddr));
      alignas(32) uint8_t buffer[512];
      const int rc =
          mdns_query_send(m_sock, MDNS_RECORDTYPE_PTR, VALETUDO_SERVICE,
                          VALETUDO_SERVICE_LEN, buffer, sizeof(buffer), 0);
      qDebug() << "mDNS: sent PTR query on" << iface.name()
               << entry.ip().toString() << (rc >= 0 ? "ok" : "failed");
    }
  }
}

void MdnsDiscovery::onSocketReady() {
  alignas(32) uint8_t buffer[RECV_BUFFER_SIZE];
  RecvContext ctx{&m_pending};
  const size_t records = mdns_query_recv(m_sock, buffer, sizeof(buffer),
                                         mdns_record_callback, &ctx, 0);
  qDebug() << "mDNS: packet received," << records << "records";

  for (auto it = m_pending.cbegin(); it != m_pending.cend(); ++it) {
    const QString& instanceName = it.key();
    const PendingService& svc = it.value();
    if (!svc.hasSrv || m_emitted.contains(instanceName)) continue;

    DiscoveredRobot robot;
    if (!svc.friendlyName.isEmpty()) {
      robot.name = svc.friendlyName;
    } else {
      robot.name = instanceName;
      if (robot.name.endsWith(VALETUDO_SERVICE_SUFFIX))
        robot.name.chop(VALETUDO_SERVICE_SUFFIX.length());
    }
    robot.id = svc.id;
    robot.model = svc.model;
    robot.manufacturer = svc.manufacturer;
    robot.version = svc.version;
    robot.host = svc.host;
    robot.port = svc.port;
    robot.address = svc.address.isNull() ? svc.senderAddr : svc.address;

    qDebug() << "mDNS: emitting robot" << robot.name << "at" << robot.url()
             << "port" << robot.port;
    emit robotDiscovered(robot);
    m_emitted.insert(instanceName);
  }
}

}  // namespace Valeronoi::robot
