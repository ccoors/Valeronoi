#ifndef VALETUDO_ROBOT_WIFI_INFORMATION_H
#define VALETUDO_ROBOT_WIFI_INFORMATION_H

#include <QJsonObject>
#include <QSharedDataPointer>

namespace Valeronoi::robot {

class WifiInformationData;

class WifiInformation {
 public:
  WifiInformation();
  WifiInformation(const QJsonObject &jsonObj);
  WifiInformation(const double signal);
  WifiInformation(const double signal, const QString ssid, const QString bssid);

  WifiInformation(const WifiInformation &);
  WifiInformation &operator=(const WifiInformation &);
  ~WifiInformation();

  void set_ssid(const QString ssid);
  void set_bssid(const QString bssid);
  void set_signal(const double signal);

  QString ssid() const;
  QString bssid() const;
  double signal() const;

  QJsonObject get_json() const;
  void set_json(QJsonObject jsonObj);

  bool has_valid_signal() const;

 private:
  QSharedDataPointer<WifiInformationData> m_data;
};

}  // namespace Valeronoi::robot
#endif  // VALETUDO_ROBOT_WIFI_INFORMATION_H
