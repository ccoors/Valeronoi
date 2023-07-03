#ifndef WIFI_INFORMATION_H
#define WIFI_INFORMATION_H

#include <QSharedDataPointer>

namespace Valeronoi::robot {

class Wifi_InformationData;

class Wifi_Information
{
public:
    Wifi_Information();
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

private:
    QSharedDataPointer<Wifi_InformationData> data;
};

}
#endif // WIFI_INFORMATION_H
