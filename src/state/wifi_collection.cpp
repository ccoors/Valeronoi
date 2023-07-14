#include "wifi_collection.h"
#include "qjsonarray.h"


namespace Valeronoi::state {

wifi_collection::wifi_collection()
{
    clear();
}

void wifi_collection::clear()
{
    m_known_wifis.clear();
    m_known_wifi_ids.clear();
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
    m_known_wifis.append(wifiInfo);
    m_known_wifi_ids.append(m_known_wifis.size()-1);

    if (m_known_wifis.size() != m_known_wifi_ids.size())
    {
        // out of sync.
        throw "Vector size missmatch! this is bad";
    }

    emit signal_wifiListChanged();
    emit signal_newWifiAdded(wifiInfo);

    return m_known_wifi_ids.last();
}

QVector<Valeronoi::robot::Wifi_Information> wifi_collection::get_known_wifis() const
{
    return m_known_wifis;
}

QJsonArray wifi_collection::get_json() const
{
    QJsonArray jArr;

    for (int index = 0; m_known_wifis.size() > index ; ++index) {
        QJsonObject wifiAp = m_known_wifis.at(index).get_json();
        wifiAp.insert("index",m_known_wifi_ids.at(index));
        jArr.append(wifiAp);
    }

    return jArr;
}

void wifi_collection::set_json(const QJsonArray &json)
{
    clear();

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
        m_known_wifi_ids.append(index);
        m_known_wifis.append(Valeronoi::robot::Wifi_Information(wifiAp));
    }

    emit signal_wifiListChanged();

    return;
}

int wifi_collection::get_wifiId(QString bssid) const
{
    int nRet = -1;

    for (int index = 0; m_known_wifis.size() > index ; ++index) {
        if (m_known_wifis.at(index).bssid() == bssid) {
            nRet = m_known_wifi_ids.at(index);
            break;
        }
    }

    return nRet;
}


}
