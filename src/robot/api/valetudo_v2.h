#ifndef VALERONOI_ROBOT_API_VALETUDO_V2_H
#define VALERONOI_ROBOT_API_VALETUDO_V2_H

#include <QByteArray>
#include <QJsonDocument>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "../commands.h"
#include "../connection_configuration.h"
#include "../robot_information.h"
#include "sse.h"

namespace Valeronoi::robot::api::v2 {

const QUrl ROBOT_WIFI_CAPABILITY =
    QUrl("/api/v2/robot/capabilities/WifiConfigurationCapability");

class ValetudoAPI : public QObject {
  Q_OBJECT
 public:
  ValetudoAPI();

  void set_connection_configuration(const ConnectionConfiguration &conf);

  [[nodiscard]] const RobotInformation *get_information() const;

  [[nodiscard]] QString get_error() const;

  [[nodiscard]] QString get_warning() const;

  [[nodiscard]] bool is_connected() const;

  [[nodiscard]] bool is_connecting() const;

  QNetworkReply *request(const QString &verb, const QUrl &url,
                         bool disconnect_on_failure = false, bool gc = false,
                         const QByteArray *data = nullptr);

  [[nodiscard]] QString get_map_data() const;

  void send_command(Valeronoi::robot::BASIC_COMMANDS command);

  void relocate(int x, int y);

 public slots:
  void slot_connect();

  void slot_disconnect();

 signals:
  void signal_connecting();

  void signal_connecting_step(float value);

  void signal_connected();

  void signal_connection_error();

  void signal_connection_ended();

  void signal_map_updated();

 private slots:
  void slot_map_updated();

 private:
  void next_connection_step();

  bool m_connected{false}, m_connecting{false};
  int m_connecting_step{0};
  RobotInformation m_robot_information;
  ConnectionConfiguration m_connection_configuration;

  QString m_error_message, m_warning_message;

  QNetworkAccessManager m_qnam;
  QList<QJsonDocument> m_connection_responses;

  Valeronoi::robot::api::SSEConnection m_map_connection;
};

}  // namespace Valeronoi::robot::api::v2

#endif
