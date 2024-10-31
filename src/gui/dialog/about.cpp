/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2024 Christian Friedrich Coors <me@ccoors.de>
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
#include "about.h"

#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>

#include "config.h"
#include "ui_about.h"

namespace Valeronoi::gui::dialog {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);
  ui->version->setText(VALERONOI_VERSION);
  ui->gitCommit->setText(VALERONOI_GIT_COMMIT);

  connect(ui->aboutQt, &QPushButton::clicked, this,
          [=]() { QMessageBox::aboutQt(this); });
}

AboutDialog::~AboutDialog() { delete ui; }

}  // namespace Valeronoi::gui::dialog
