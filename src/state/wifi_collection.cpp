#include "wifi_collection.h"
#include "qjsonarray.h"


namespace Valeronoi::state {

wifi_collection::wifi_collection()
{

}

int wifi_collection::get_or_create_wifiId(Valeronoi::robot::Wifi_Information wifiInfo)
{
    (void)wifiInfo;
    return -1;
}

int wifi_collection::add_wifi(Valeronoi::robot::Wifi_Information wifiInfo)
{
    (void)wifiInfo;
    return -1;
}

QVector<Valeronoi::robot::Wifi_Information> wifi_collection::get_known_wifis() const
{
    return m_knownWifis;
}

QJsonArray wifi_collection::get_json() const
{
    return QJsonArray();
}

void wifi_collection::set_json(const QJsonArray &json)
{
    (void)json;
    return;
}

int wifi_collection::get_wifiId(QString bssid) const
{
    int nRet = -1;

    (void) bssid;
    return nRet;
}


}
