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

int wifi_collection::get_or_create_wifi_id(Valeronoi::robot::WifiInformation wifi_info)
{
    int nRet = get_wifi_id(wifi_info.bssid());

    if (nRet < 0) {
        nRet = add_wifi(wifi_info);
    }

    return nRet;
}

int wifi_collection::add_wifi(Valeronoi::robot::WifiInformation wifi_info)
{
    m_known_wifis.append(wifi_info);
    m_known_wifi_ids.append(m_known_wifis.size()-1);

    if (m_known_wifis.size() != m_known_wifi_ids.size())
    {
        // out of sync.
        throw "Vector size missmatch! this is bad";
    }

    emit signal_wifi_list_updated();
    emit signal_new_wifi_added(wifi_info);

    return m_known_wifi_ids.last();
}

QVector<Valeronoi::robot::WifiInformation> wifi_collection::get_known_wifis() const
{
    return m_known_wifis;
}

QJsonArray wifi_collection::get_json() const
{
    QJsonArray jArr;

    for (int index = 0; m_known_wifis.size() > index ; ++index) {
        QJsonObject wifi_access_point = m_known_wifis.at(index).get_json();
        wifi_access_point.insert("index",m_known_wifi_ids.at(index));
        jArr.append(wifi_access_point);
    }

    return jArr;
}

void wifi_collection::set_json(const QJsonArray &json)
{
    clear();

    foreach (auto wifi_access_point_value, json) {
        if (!wifi_access_point_value.isObject()) {
            // Wrong format.
        }
        QJsonObject wifi_access_point = wifi_access_point_value.toObject();
        int index = wifi_access_point["index"].toInt(-1);
        if (index < 0) {
            // error index.
            // continue Mapping this as -1.
        }
        m_known_wifi_ids.append(index);
        m_known_wifis.append(Valeronoi::robot::WifiInformation(wifi_access_point));
    }

    emit signal_wifi_list_updated();

    return;
}

int wifi_collection::get_wifi_id(QString bssid) const
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
