#include "wifi_information.h"

namespace Valeronoi::robot {

class WifiInformationData : public QSharedData
{
public:
    WifiInformationData() {
        m_bssid = "00:00:00:00:00:00";
        m_ssid = "Unknown Wifi";
        m_signal = 0.0;
    }

    WifiInformationData(const WifiInformationData &other)
        : QSharedData(other),
        m_ssid(other.m_ssid),
        m_bssid(other.m_bssid),
        m_signal(other.m_signal)
    { }

    ~WifiInformationData() { }

    QString m_ssid;
    QString m_bssid;

    double m_signal; // measurement

//    // Optionally this could be expanded
//    QString m_frequency;
//    QString m_ip;
//    QString m_ipv6;
//    double m_upspeed; // Measurement
};

WifiInformation::WifiInformation() : m_data(new WifiInformationData)
{

}

WifiInformation::WifiInformation(const QJsonObject &jsonObj) : m_data(new WifiInformationData)
{
    set_json(jsonObj);
}

WifiInformation::WifiInformation(const double signal)
    : m_data(new WifiInformationData)
{
    set_signal(signal);
}

WifiInformation::WifiInformation(const double signal, const QString ssid, const QString bssid)
    : m_data(new WifiInformationData)
{
    set_signal(signal);
    set_ssid(ssid);
    set_bssid(bssid);
}

WifiInformation::WifiInformation(const WifiInformation &other)
    : m_data(other.m_data)
{
}

WifiInformation &WifiInformation::operator=(const WifiInformation &rhs)
{
    if (this != &rhs)
        m_data.operator=(rhs.m_data);
    return *this;
}

WifiInformation::~WifiInformation()
{

}

void WifiInformation::set_ssid(const QString ssid)
{
    m_data->m_ssid = ssid;
}

void WifiInformation::set_bssid(const QString bssid)
{
    m_data->m_bssid = bssid;
}

void WifiInformation::set_signal(const double signal)
{
    m_data->m_signal = signal;
}

QString WifiInformation::ssid() const
{
    return m_data->m_ssid;
}

QString WifiInformation::bssid() const
{
    return m_data->m_bssid;
}

double WifiInformation::signal() const
{
    return m_data->m_signal;
}

QJsonObject WifiInformation::get_json() const
{
    QJsonObject retObj;

    retObj.insert("bssid",m_data->m_bssid);
    retObj.insert("ssid",m_data->m_ssid);
    retObj.insert("signal",m_data->m_signal);

    return retObj;
}

void WifiInformation::set_json(QJsonObject jsonObj)
{
    if (jsonObj.contains("signal") && jsonObj["signal"].isDouble()) {
        m_data->m_signal = jsonObj["signal"].toDouble();
    }

    if (jsonObj.contains("bssid") && jsonObj["bssid"].isString()) {
        m_data->m_bssid = jsonObj["bssid"].toString();
    }

    if (jsonObj.contains("ssid") && jsonObj["ssid"].isString()) {
        m_data->m_ssid = jsonObj["ssid"].toString();
    }

}

bool WifiInformation::has_valid_signal() const
{
    return (m_data->m_signal < -1);
}

}
