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
#ifndef VALERONOI_H
#define VALERONOI_H

#include <QColorDialog>
#include <QMainWindow>
#include <QSettings>
#include <QtWidgets>
#include <array>
#include <map>
#include <string>

#include "gui/dialog/about.h"
#include "gui/dialog/export.h"
#include "gui/dialog/log.h"
#include "gui/dialog/robot_config.h"
#include "gui/dialog/settings.h"
#include "gui/dialog/update.h"
#include "gui/widget/display_widget.h"
#include "robot/connection_configuration.h"
#include "robot/robot.h"
#include "state/measurements.h"
#include "state/robot_map.h"
#include "util/colormap.h"
#include "util/log_helper.h"

namespace Valeronoi {
constexpr const char *VALERONOI_FILE_EXTENSION = "vwm";  // Valeronoi WiFi Map
constexpr const int FILE_FORMAT_VERSION = 1;

QT_BEGIN_NAMESPACE
namespace Ui {
class ValeronoiWindow;
}
QT_END_NAMESPACE

class ValeronoiWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit ValeronoiWindow(QWidget *parent = nullptr);

  ~ValeronoiWindow() override;

  bool load_file(const QString &path);

  void closeEvent(QCloseEvent *event) override;

 private slots:
  void newFile();

  void openFile();

  void saveFile();

  void saveAsFile();

  void slot_begin_recording();

  void slot_end_recording();

  void slot_toggle_recording();

 private:
  void update_title();

  [[nodiscard]] bool maybe_save();

  [[nodiscard]] bool save();

  void load_colormaps();

  void set_modified(bool is_modified);

  [[nodiscard]] QString get_open_save_dir() const;

  void set_open_save_dir(const QString &dir);

  [[nodiscard]] QString get_open_save_filter() const;

  void update_display_settings();

  void connect_display_widget();

  void connect_actions();

  Ui::ValeronoiWindow *ui;
  Valeronoi::gui::widget::DisplayWidget *m_display_widget;

  Valeronoi::gui::dialog::ExportDialog m_export_dialog;
  Valeronoi::gui::dialog::RobotConfigDialog m_robot_config_dialog;
  Valeronoi::gui::dialog::SettingsDialog m_settings_dialog;
  Valeronoi::gui::dialog::AboutDialog m_about_dialog;
  Valeronoi::gui::dialog::UpdateDialog m_update_dialog;
  Valeronoi::gui::dialog::LogDialog m_log_dialog;

  bool m_modified;
  QString m_current_file;

  QList<Valeronoi::util::RGBColorMap> m_color_maps;

  Valeronoi::robot::ConnectionConfiguration m_connection_configuration;
  Valeronoi::robot::Robot m_robot;

  Valeronoi::state::RobotMap m_robot_map;
  Valeronoi::state::Measurements m_wifi_measurements;
  bool m_recording{false};
  void connect_robot_signals();
};

}  // namespace Valeronoi

#endif
