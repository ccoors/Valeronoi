/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2022 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_GUI_DIALOG_ROBOT_CONFIG_H
#define VALERONOI_GUI_DIALOG_ROBOT_CONFIG_H

#include <QDialog>
#include <QJsonDocument>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSettings>
#include <QUrl>
#include <QtWidgets>

#include "../../robot/robot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class RobotConfigDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {
class RobotConfigDialog : public QDialog {
  Q_OBJECT
 public:
  explicit RobotConfigDialog(QWidget *parent = nullptr);

  ~RobotConfigDialog() override;

  void load_settings();

 signals:
  void signal_config_changed();

 private slots:
  void slot_save();

  void slot_test_connection();

  void slot_end_test();

  void slot_robot_connected();

  void slot_robot_connection_failed();

 private:
  void ensure_http();

  Ui::RobotConfigDialog *ui;
  QProgressDialog m_progress_dialog;
  bool m_test_cancelled{false};

  Valeronoi::robot::Robot m_robot;
};
}  // namespace Valeronoi::gui::dialog

#endif
