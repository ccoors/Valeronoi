#ifndef VALETUDO_ROBOT_WIFI_INFORMATION_H
#define VALETUDO_ROBOT_WIFI_INFORMATION_H

#include <QSharedDataPointer>
#include <QJsonObject>

namespace Valeronoi::robot {

class Wifi_InformationData;

class Wifi_Information
{
public:
    Wifi_Information();
    Wifi_Information(const QJsonObject& jsonObj);
    Wifi_Information(const double signal);
    Wifi_Information(const double signal, const QString ssid, const QString bssid);

    Wifi_Information(const Wifi_Information &);
    Wifi_Information &operator=(const Wifi_Information &);
    ~Wifi_Information();

    void setSsid(const QString ssid);
    void setBssid(const QString bssid);
    void setSignal(const double signal);

    QString ssid() const;
    QString bssid() const;
    double signal() const;

    QJsonObject get_json() const;
    void set_json(QJsonObject jsonObj);

    bool has_valid_signal() const;
private:
    QSharedDataPointer<Wifi_InformationData> m_data;
};

}
#endif // VALETUDO_ROBOT_WIFI_INFORMATION_H
