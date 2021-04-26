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
#include "robot_config.h"

#include <QMessageBox>
#include <QTextStream>

#include "../../robot/connection_configuration.h"
#include "ui_robot_config.h"

namespace Valeronoi::gui::dialog {
RobotConfigDialog::RobotConfigDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::RobotConfigDialog),
      m_progress_dialog(tr("Testing connection..."), tr("Cancel"), 0, 100,
                        this) {
  ui->setupUi(this);
  setWindowFlags(Qt::Sheet);

  m_progress_dialog.setWindowModality(Qt::ApplicationModal);
  m_progress_dialog.setAutoClose(true);
  m_progress_dialog.close();
  connect(&m_progress_dialog, &QProgressDialog::canceled, this,
          &RobotConfigDialog::slot_end_test);

  connect(this, &QDialog::accepted, this, &RobotConfigDialog::slot_save);
  connect(ui->buttonTestConnection, &QPushButton::clicked, this,
          &RobotConfigDialog::slot_test_connection);

  connect(&m_robot, &Valeronoi::robot::Robot::signal_connecting_step, this,
          [=](float value) {
            qDebug().nospace() << "Received connection step " << value;
            if (!m_test_cancelled) {
              m_progress_dialog.setValue(10 + static_cast<int>(80 * value));
            }
          });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connected, this,
          &RobotConfigDialog::slot_robot_connected);
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connection_error, this,
          &RobotConfigDialog::slot_robot_connection_failed);

  load_settings();
}

RobotConfigDialog::~RobotConfigDialog() { delete ui; }

void RobotConfigDialog::slot_save() {
  QSettings settings;
  ensure_http();

  settings.setValue("robot/url", ui->valetudoAddress->text());
  settings.setValue("robot/auth/enabled", ui->groupBoxAuth->isChecked());
  settings.setValue("robot/auth/username", ui->authUsername->text());
  settings.setValue("robot/auth/password", ui->authPassword->text());

  emit signal_config_changed();
}

void RobotConfigDialog::load_settings() {
  QSettings settings;
  ui->valetudoAddress->setText(
      settings.value("robot/url", "http://").toString());
  ui->groupBoxAuth->setChecked(
      settings.value("robot/auth/enabled", false).toBool());
  ui->authUsername->setText(
      settings.value("robot/auth/username", "").toString());
  ui->authPassword->setText(
      settings.value("robot/auth/password", "").toString());

  ensure_http();
}

void RobotConfigDialog::slot_end_test() {
  m_test_cancelled = true;
  m_robot.slot_disconnect();
  m_progress_dialog.close();
}

void RobotConfigDialog::slot_test_connection() {
  ensure_http();
  const QString url_input = ui->valetudoAddress->text().trimmed();
  if (url_input.isEmpty()) {
    QMessageBox::warning(this, tr("Error"),
                         tr("Please enter the Valetudo address"));
    return;
  }

  QUrl base_url = QUrl::fromUserInput(url_input);
  if (!base_url.isValid()) {
    QMessageBox::warning(
        this, tr("Error"),
        tr("Invalid URL: %1: %2").arg(url_input, base_url.errorString()));
    return;
  }

  m_test_cancelled = false;
  m_progress_dialog.setValue(10);
  Valeronoi::robot::ConnectionConfiguration c;
  c.m_url = base_url;
  c.m_auth = ui->groupBoxAuth->isChecked();
  c.m_username = ui->authUsername->text();
  c.m_password = ui->authPassword->text();
  m_robot.set_connection_configuration(c);
  m_robot.slot_connect();
}

void RobotConfigDialog::slot_robot_connected() {
  m_progress_dialog.close();
  const auto robot_information = m_robot.get_information();
  QString robot_description = QString("%1 / %2 (%3)")
                                  .arg(robot_information->m_manufacturer)
                                  .arg(robot_information->m_model_name)
                                  .arg(robot_information->m_implementation);

  bool has_wifi_config =
      robot_information->m_capabilities.contains("WifiConfigurationCapability");
  bool has_map_save_capability = robot_information->m_capabilities.contains(
      "PersistentMapControlCapability");
  bool map_saving_enabled =
      robot_information
          ->get_attribute("PersistentMapSettingStateAttribute")["value"]
          .toString() == "enabled";

  QString has_wifi_config_description, has_map_save_description;
  if (has_wifi_config) {
    has_wifi_config_description =
        tr("✅ The robot has the WifiConfigurationCapability.");
  } else {
    has_wifi_config_description =
        tr("❌ The robot does not have the WifiConfigurationCapability.");
  }

  if (map_saving_enabled) {
    has_map_save_description =
        tr("✅ The robot has the PersistentMapSettingStateAttribute and it is "
           "enabled.");
  } else {
    if (has_map_save_capability) {
      has_map_save_description =
          tr("❌ The robot has the PersistentMapControlCapability, but the "
             "PersistentMapSettingStateAttribute is not enabled.");
    } else {
      has_map_save_description = tr(
          "❌ The robot does not have the PersistentMapControlCapability! Make "
          "sure to not do full cleans while recording.");
    }
  }

  m_robot.slot_disconnect();

  QString test_result =
      tr("Connection successful. Detected:\n%1\nRunning Valetudo %2\n\n%3\n%4")
          .arg(robot_description)
          .arg(robot_information->m_valetudo_version)
          .arg(has_wifi_config_description)
          .arg(has_map_save_description);

  if (has_wifi_config) {
    QMessageBox::information(this, tr("Test successful"), test_result);
  } else {
    QMessageBox::warning(this, tr("Test failed"), test_result);
  }
}

void RobotConfigDialog::slot_robot_connection_failed() {
  m_progress_dialog.close();
  QMessageBox::warning(this, tr("Error"),
                       tr("Test failed:\n%1").arg(m_robot.get_error()));
}

void RobotConfigDialog::ensure_http() {
  const auto current_text = ui->valetudoAddress->text().trimmed();
  if (!current_text.startsWith("http://")) {
    ui->valetudoAddress->setText(QString("http://").append(current_text));
  }
}
}  // namespace Valeronoi::gui::dialog
