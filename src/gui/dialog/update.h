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
#ifndef VALERONOI_GUI_DIALOG_UPDATE_H
#define VALERONOI_GUI_DIALOG_UPDATE_H

#include <QDialog>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
class UpdateDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {

class UpdateDialog : public QDialog {
  Q_OBJECT
 public:
  explicit UpdateDialog(QWidget *parent = nullptr);

  ~UpdateDialog() override;

  void check_update(bool silent = false, QWidget *error_parent = nullptr);

 private:
  Ui::UpdateDialog *ui;

  QUrl m_goto_url;

  QNetworkAccessManager m_qnam;
};

}  // namespace Valeronoi::gui::dialog

#endif
