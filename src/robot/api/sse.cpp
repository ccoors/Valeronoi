#include "sse.h"

namespace Valeronoi::robot::api {

SSEConnection::SSEConnection() {
  m_reconnect_timer.setSingleShot(true);
  connect(&m_reconnect_timer, &QTimer::timeout, this,
          &SSEConnection::slot_make_request);
}

void SSEConnection::set_connection_configuration(
    const ConnectionConfiguration &conf) {
  m_connection_configuration = conf;
}

void SSEConnection::set_url(const QUrl &url) { m_url = url; }

QString SSEConnection::current_data() const { return m_current_data; }

void SSEConnection::slot_connect() {
  if (m_connected) {
    return;
  }
  m_connected = true;
  slot_make_request();
}

void SSEConnection::slot_ready_read() {
  if (!m_reply) {
    // Should not happen
    return;
  }
  // m_reply->readAll returns chunked data, so we put it back together and
  // parse it according to https://www.w3.org/TR/eventsource/
  const auto buffer = m_reply->readAll();
  qDebug() << "Received SSE data: size" << buffer.size();
  QStringList lines = QString(buffer).split("\n");

  for (const QString &raw_line : lines) {
    QString line = raw_line.simplified();

    switch (m_parser_state) {
      case ParserState::IDLE:
        if (line.startsWith(":")) {
          continue;
        } else if (line.startsWith("event: ")) {
          m_event_type = line.right(line.size() - 7);
        } else if (line.startsWith("data: ") && m_event_type == m_event) {
          m_data_buffer = line.right(line.size() - 6);
          m_parser_state = ParserState::DATA;
        }
        break;
      case ParserState::DATA:
        if (line.isEmpty()) {
          m_parser_state = ParserState::IDLE;
          m_current_data = m_data_buffer;
          emit signal_data_updated();
        } else {
          m_data_buffer.append(line);
        }
    }
  }
}

void SSEConnection::slot_disconnect() {
  if (!m_connected) {
    return;
  }
  m_connected = false;
  if (m_reply) {
    m_reply->abort();
    m_reply = nullptr;
  }
}

void SSEConnection::slot_stream_finished() {
  m_reply->deleteLater();
  m_reply = nullptr;
  if (m_connected) {
    m_reconnect_timer.start(RECONNECT_TIMER);
  }
}

void SSEConnection::slot_make_request() {
  if (!m_connected) {
    return;
  }
  QNetworkRequest request = prepare_request(m_url);

  m_reply = m_qnam.get(request);
  connect(m_reply, &QNetworkReply::readyRead, this,
          &SSEConnection::slot_ready_read);
  connect(m_reply, &QNetworkReply::finished, this,
          &SSEConnection::slot_stream_finished);
}

QNetworkRequest SSEConnection::prepare_request(const QUrl &url) const {
  QNetworkRequest request(m_connection_configuration.m_url.resolved(url));
  m_connection_configuration.prepare_request(request);
  request.setRawHeader("Accept", "text/event-stream");
  return request;
}

void SSEConnection::set_event(const QString &event) { m_event = event; }

}  // namespace Valeronoi::robot::api
