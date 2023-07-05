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

    int get_wifiId();

    QVector<Valeronoi::robot::Wifi_Information> get_known_wifis();

    QJsonObject get_Json();
    void set_Json(QJsonObject jsonObj);

protected:

private:
    QVector<Valeronoi::robot::Wifi_Information> m_knownWifis;
};

}

#endif // WIFI_COLLECTION_H
