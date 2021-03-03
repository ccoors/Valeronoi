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
