#ifndef WIFI_COLLECTION_H
#define WIFI_COLLECTION_H

#include <QVector>
#include <QJsonObject>
#include "../robot/wifi_information.h"

namespace Valeronoi::state {

class wifi_collection
{
public:
    wifi_collection();

    int get_or_create_wifiId(Valeronoi::robot::Wifi_Information wifiInfo);

    int add_wifi(Valeronoi::robot::Wifi_Information wifiInfo);

    QVector<Valeronoi::robot::Wifi_Information> get_known_wifis() const;

    QJsonArray get_json() const;
    void set_json(const QJsonArray &json);

protected:
    int get_wifiId(QString bssid) const;

private:
    QVector<Valeronoi::robot::Wifi_Information> m_knownWifis;
    QVector<int> m_knownWifiIds;
};

}

#endif // WIFI_COLLECTION_H
