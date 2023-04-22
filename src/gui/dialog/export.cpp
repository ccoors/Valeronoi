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
#include "export.h"

#include "ui_export.h"

namespace Valeronoi::gui::dialog {
ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExportDialog) {
  ui->setupUi(this);
  setWindowFlags(Qt::Sheet);

  connect(ui->width, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [=](int width) {
            int new_height = static_cast<int>(width / m_ratio);
            if (ui->height->value() != new_height) {
              const bool block = ui->height->blockSignals(true);
              ui->height->setValue(new_height);
              ui->height->blockSignals(block);
            }
          });

  connect(ui->height, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [=](int height) {
            int new_width = static_cast<int>(height * m_ratio);
            if (ui->width->value() != new_width) {
              const bool block = ui->width->blockSignals(true);
              ui->width->setValue(new_width);
              ui->width->blockSignals(block);
            }
          });
}

ExportDialog::~ExportDialog() { delete ui; }

void ExportDialog::set_size(const QSize &size) {
  ui->width->setMaximum(10 * size.width());
  ui->height->setMaximum(10 * size.height());
  const bool block_w = ui->width->blockSignals(true);
  const bool block_h = ui->height->blockSignals(true);
  ui->width->setValue(size.width());
  ui->height->setValue(size.height());
  ui->width->blockSignals(block_w);
  ui->height->blockSignals(block_h);
  m_ratio =
      static_cast<double>(size.width()) / static_cast<double>(size.height());
  ui->transparent->setChecked(true);
}

QSize ExportDialog::get_size() const {
  return QSize(ui->width->value(), ui->height->value());
}

bool ExportDialog::get_transparent() const {
  return ui->transparent->isChecked();
}

}  // namespace Valeronoi::gui::dialog
