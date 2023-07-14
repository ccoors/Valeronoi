#include "wifi_information.h"

namespace Valeronoi::robot {

class Wifi_InformationData : public QSharedData
{
public:
    Wifi_InformationData() {
        m_bssid = "00:00:00:00:00:00";
        m_ssid = "Unknown Wifi";
        m_signal = 0.0;
    }

    Wifi_InformationData(const Wifi_InformationData &other)
        : QSharedData(other),
        m_ssid(other.m_ssid),
        m_bssid(other.m_bssid),
        m_signal(other.m_signal)
    { }

    ~Wifi_InformationData() { }

    QString m_ssid;
    QString m_bssid;

    double m_signal; // measurement

//    // Optionally this could be expanded
//    QString m_frequency;
//    QString m_ip;
//    QString m_ipv6;
//    double m_upspeed; // Measurement
};

Wifi_Information::Wifi_Information() : m_data(new Wifi_InformationData)
{

}

Wifi_Information::Wifi_Information(const QJsonObject &jsonObj) : m_data(new Wifi_InformationData)
{
    set_json(jsonObj);
}

Wifi_Information::Wifi_Information(const double signal)
    : m_data(new Wifi_InformationData)
{
    setSignal(signal);
}

Wifi_Information::Wifi_Information(const double signal, const QString ssid, const QString bssid)
    : m_data(new Wifi_InformationData)
{
    setSignal(signal);
    setSsid(ssid);
    setBssid(bssid);
}

Wifi_Information::Wifi_Information(const Wifi_Information &other)
    : m_data(other.m_data)
{
}

Wifi_Information &Wifi_Information::operator=(const Wifi_Information &rhs)
{
    if (this != &rhs)
        m_data.operator=(rhs.m_data);
    return *this;
}

Wifi_Information::~Wifi_Information()
{

}

void Wifi_Information::setSsid(const QString ssid)
{
    m_data->m_ssid = ssid;
}

void Wifi_Information::setBssid(const QString bssid)
{
    m_data->m_bssid = bssid;
}

void Wifi_Information::setSignal(const double signal)
{
    m_data->m_signal = signal;
}

QString Wifi_Information::ssid() const
{
    return m_data->m_ssid;
}

QString Wifi_Information::bssid() const
{
    return m_data->m_bssid;
}

double Wifi_Information::signal() const
{
    return m_data->m_signal;
}

QJsonObject Wifi_Information::get_json() const
{
    QJsonObject retObj;

    retObj.insert("bssid",m_data->m_bssid);
    retObj.insert("ssid",m_data->m_ssid);
    retObj.insert("signal",m_data->m_signal);

    return retObj;
}

void Wifi_Information::set_json(QJsonObject jsonObj)
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

bool Wifi_Information::has_valid_signal() const
{
    return (m_data->m_signal < -1);
}

}
