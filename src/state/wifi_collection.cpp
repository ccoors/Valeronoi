#include "wifi_collection.h"
#include "qjsonarray.h"


namespace Valeronoi::state {

wifi_collection::wifi_collection()
{
    m_knownWifis.clear();
    m_knownWifiIds.clear();
}

int wifi_collection::get_or_create_wifiId(Valeronoi::robot::Wifi_Information wifiInfo)
{
    int nRet = get_wifiId(wifiInfo.bssid());

    if (nRet < 0) {
        nRet = add_wifi(wifiInfo);
    }

    return nRet;
}

int wifi_collection::add_wifi(Valeronoi::robot::Wifi_Information wifiInfo)
{
    m_knownWifis.append(wifiInfo);
    m_knownWifiIds.append(m_knownWifis.size()-1);

    if (m_knownWifis.size() != m_knownWifiIds.size())
    {
        // out of sync.
        throw "Vector size missmatch! this is bad";
    }

    emit signal_wifiListChanged();
    emit signal_newWifiAdded(wifiInfo);

    return m_knownWifiIds.last();
}

QVector<Valeronoi::robot::Wifi_Information> wifi_collection::get_known_wifis() const
{
    return m_knownWifis;
}

QJsonArray wifi_collection::get_json() const
{
    QJsonArray jArr;

    for (int index = 0; m_knownWifis.size() > index ; ++index) {
        QJsonObject wifiAp = m_knownWifis.at(index).get_json();
        wifiAp.insert("index",m_knownWifiIds.at(index));
        jArr.append(wifiAp);
    }

    return jArr;
}

void wifi_collection::set_json(const QJsonArray &json)
{
    m_knownWifiIds.clear();
    m_knownWifis.clear();

    foreach (auto wifiApVal, json) {
        if (!wifiApVal.isObject()) {
            // Wrong format.
        }
        QJsonObject wifiAp = wifiApVal.toObject();
        int index = wifiAp["index"].toInt(-1);
        if (index < 0) {
            // error index.
            // continue Mapping this as -1.
        }
        m_knownWifiIds.append(index);
        m_knownWifis.append(Valeronoi::robot::Wifi_Information(wifiAp));
    }

    emit signal_wifiListChanged();

    return;
}

int wifi_collection::get_wifiId(QString bssid) const
{
    int nRet = -1;

    for (int index = 0; m_knownWifis.size() > index ; ++index) {
        if (m_knownWifis.at(index).bssid() == bssid) {
            nRet = m_knownWifiIds.at(index);
            break;
        }
    }

    return nRet;
}


}
