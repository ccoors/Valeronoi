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
#include "update.h"

#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "config.h"
#include "ui_update.h"

namespace Valeronoi::gui::dialog {

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::UpdateDialog) {
  ui->setupUi(this);
  ui->currentVersion->setText(VALERONOI_VERSION);

  connect(ui->goToRelease, &QPushButton::clicked, this,
          [=]() { QDesktopServices::openUrl(m_goto_url); });
}

UpdateDialog::~UpdateDialog() { delete ui; }

void UpdateDialog::check_update(bool silent, QWidget *error_parent) {
  qDebug() << "Checking for updates";

  auto r = QNetworkRequest(
      QUrl("https://api.github.com/repos/ccoors/Valeronoi/releases"));
  r.setTransferTimeout(10000);
  r.setMaximumRedirectsAllowed(10);
  r.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                 QNetworkRequest::AlwaysNetwork);
  r.setHeader(QNetworkRequest::UserAgentHeader, "Valeronoi/" VALERONOI_VERSION);
  auto reply = m_qnam.get(r);
  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() != QNetworkReply::NoError) {
      QMessageBox::warning(
          error_parent, "Valeronoi",
          tr("Update check failed: %1").arg(reply->errorString()));
    } else {
      auto data = reply->readAll();
      QJsonParseError error;
      auto json = QJsonDocument::fromJson(data, &error);
      if (json.isNull()) {
        QMessageBox::warning(
            error_parent, "Valeronoi",
            tr("Error parsing update data: %1").arg(error.errorString()));
      } else {
        auto newest_release = json.array()[0].toObject();
        auto current_tag = newest_release["tag_name"].toString();
        m_goto_url = newest_release["html_url"].toString();
        if (current_tag.isEmpty() || m_goto_url.isEmpty()) {
          QMessageBox::information(error_parent, "Valeronoi",
                                   tr("Could not load update information"));
          return;
        }
        ui->updatedVersion->setText(current_tag);
        if (current_tag != "v" VALERONOI_VERSION) {
          open();
          raise();
        } else if (!silent) {
          QMessageBox::information(error_parent, "Valeronoi",
                                   tr("No new version available"));
        }
      }
    }
    reply->deleteLater();
  });
}

}  // namespace Valeronoi::gui::dialog
