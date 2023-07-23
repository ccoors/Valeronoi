#ifndef VALERONOI_STATE_WIFI_COLLECTION_H
#define VALERONOI_STATE_WIFI_COLLECTION_H

#include <QJsonObject>
#include <QObject>
#include <QVector>

#include "../robot/wifi_information.h"

namespace Valeronoi::state {

class wifi_collection : public QObject {
  Q_OBJECT
 public:
  wifi_collection();

  void clear();

  int get_or_create_wifi_id(Valeronoi::robot::WifiInformation wifi_info);

  int get_wifi_id(QString bssid) const;

  int add_wifi(Valeronoi::robot::WifiInformation wifi_info);

  QVector<Valeronoi::robot::WifiInformation> get_known_wifis() const;

  QJsonArray get_json() const;
  void set_json(const QJsonArray &json);

 signals:
  void signal_wifi_list_updated();
  void signal_new_wifi_added(Valeronoi::robot::WifiInformation wifi_info);

 protected:
 private:
  QVector<Valeronoi::robot::WifiInformation> m_known_wifis;
  QVector<int> m_known_wifi_ids;
};

}  // namespace Valeronoi::state

#endif  // VALERONOI_STATE_WIFI_COLLECTION_H
