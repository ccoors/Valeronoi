/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_GUI_DIALOG_ABOUT_H
#define VALERONOI_GUI_DIALOG_ABOUT_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AboutDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {

class AboutDialog : public QDialog {
  Q_OBJECT
 public:
  explicit AboutDialog(QWidget *parent = nullptr);

  ~AboutDialog() override;

 private:
  Ui::AboutDialog *ui;
};

}  // namespace Valeronoi::gui::dialog

#endif
