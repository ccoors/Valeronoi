/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2023 Christian Friedrich Coors <me@ccoors.de>
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
#include "settings.h"

#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

#include "ui_settings.h"

namespace Valeronoi::gui::dialog {
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);
  setWindowFlags(Qt::Sheet);

  connect(ui->checkUpdates, &QCheckBox::checkStateChanged, this, [=]() {
    QSettings settings;
    settings.setValue("app/autoUpdateCheck", ui->checkUpdates->isChecked());
  });

  QSettings settings;
  ui->checkUpdates->setChecked(
      settings.value("app/autoUpdateCheck", false).toBool());
}

SettingsDialog::~SettingsDialog() { delete ui; }

bool SettingsDialog::should_auto_check_for_updates(QWidget *parent) {
  QSettings settings;
  bool check_for_updates{false};
  auto update_check = settings.value("app/autoUpdateCheck");
  if (update_check.isNull()) {
    auto start_count = settings.value("app/startCount", 0).toInt();
    if (start_count >= 3) {
      const auto clicked_button = QMessageBox::question(
          parent, tr("Auto Updates"),
          tr("Do you want Valeronoi to automatically check for updates?"));
      check_for_updates = (clicked_button == QMessageBox::StandardButton::Yes);
      settings.setValue("app/autoUpdateCheck", check_for_updates);
      ui->checkUpdates->setChecked(check_for_updates);
    }
  } else {
    check_for_updates = update_check.toBool();
  }
  return check_for_updates;
}

}  // namespace Valeronoi::gui::dialog
