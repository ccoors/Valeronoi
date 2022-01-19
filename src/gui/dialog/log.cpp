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
#include "log.h"

#include "ui_log.h"

namespace Valeronoi::gui::dialog {

LogDialog::LogDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LogDialog) {
  ui->setupUi(this);

  ui->logText->setFontFamily("Source Code Pro");
}

LogDialog::~LogDialog() { delete ui; }

void LogDialog::log_message(const QString &msg) { ui->logText->append(msg); }

}  // namespace Valeronoi::gui::dialog
