#include "wifi_information.h"

namespace Valeronoi::robot {

class Wifi_InformationData : public QSharedData
{
public:
    Wifi_InformationData() { }

    Wifi_InformationData(const Wifi_InformationData &other)
        : QSharedData(other),
        m_ssid(other.m_ssid),
        m_bssid(other.m_bssid),
        m_signal(other.m_signal)
    { }

    ~Wifi_InformationData() { }

    QString m_ssid;
    QString m_bssid;

    double m_signal;

//    // Optional this could be expanded
//    QString m_frequency;
//    QString m_ip;
//    QString m_ipv6;
//    double m_upspeed;
};

Wifi_Information::Wifi_Information() : data(new Wifi_InformationData)
{

}

Wifi_Information::Wifi_Information(const double signal)
    : data(new Wifi_InformationData)
{
    setSignal(signal);
}

Wifi_Information::Wifi_Information(const double signal, const QString ssid, const QString bssid)
    : data(new Wifi_InformationData)
{
    setSignal(signal);
    setSsid(ssid);
    setBssid(bssid);
}

Wifi_Information::Wifi_Information(const Wifi_Information &other)
    : data(other.data)
{
}

Wifi_Information &Wifi_Information::operator=(const Wifi_Information &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

Wifi_Information::~Wifi_Information()
{

}

void Wifi_Information::setSsid(const QString ssid)
{
    data->m_ssid = ssid;
}

void Wifi_Information::setBssid(const QString bssid)
{
    data->m_bssid = bssid;
}

void Wifi_Information::setSignal(const double signal)
{
    data->m_signal = signal;
}

QString Wifi_Information::ssid() const
{
    return data->m_ssid;
}

QString Wifi_Information::bssid() const
{
    return data->m_bssid;
}

double Wifi_Information::signal() const
{
    return data->m_signal;
}

}
