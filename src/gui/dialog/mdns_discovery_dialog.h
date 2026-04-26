/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VALERONOI_GUI_DIALOG_MDNS_DISCOVERY_DIALOG_H
#define VALERONOI_GUI_DIALOG_MDNS_DISCOVERY_DIALOG_H

#include <QDialog>
#include <QTableWidget>

#include "../../robot/mdns_discovery.h"

namespace Valeronoi::gui::dialog {

class MdnsDiscoveryDialog : public QDialog {
  Q_OBJECT
 public:
  explicit MdnsDiscoveryDialog(QWidget* parent = nullptr);

  [[nodiscard]] QString selectedUrl() const;

 private slots:
  void slot_robot_discovered(const Valeronoi::robot::DiscoveredRobot& robot);

 private:
  QTableWidget* m_table;
  Valeronoi::robot::MdnsDiscovery m_discovery;
};

}  // namespace Valeronoi::gui::dialog

#endif  // VALERONOI_GUI_DIALOG_MDNS_DISCOVERY_DIALOG_H
