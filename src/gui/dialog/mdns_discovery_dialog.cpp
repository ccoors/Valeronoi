/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021-2026 Christian Friedrich Coors <me@ccoors.de>
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
#include "mdns_discovery_dialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace Valeronoi::gui::dialog {
static constexpr int COL_NAME = 0;
static constexpr int COL_ID = 1;
static constexpr int COL_URL = 2;
static constexpr int COL_MANUFACTURER = 3;
static constexpr int COL_MODEL = 4;
static constexpr int COL_VERSION = 5;
static constexpr int NUM_COLS = 6;

MdnsDiscoveryDialog::MdnsDiscoveryDialog(QWidget* parent)
    : QDialog(parent), m_table(new QTableWidget(0, NUM_COLS, this)) {
  setWindowTitle(tr("Auto-discover Robots"));
  setMinimumSize(850, 400);

  m_table->setHorizontalHeaderLabels({tr("Name"), tr("ID"), tr("URL"),
                                      tr("Manufacturer"), tr("Model"),
                                      tr("Version")});
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->verticalHeader()->setVisible(false);
  m_table->horizontalHeader()->setSectionResizeMode(COL_NAME,
                                                    QHeaderView::Stretch);
  m_table->horizontalHeader()->setSectionResizeMode(
      COL_ID, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
      COL_URL, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
      COL_MANUFACTURER, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
      COL_MODEL, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
      COL_VERSION, QHeaderView::ResizeToContents);

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(
      new QLabel(tr("Searching for Valetudo robots on the network..."), this));
  layout->addWidget(m_table);

  auto* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
  layout->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_table, &QTableWidget::itemSelectionChanged, this, [=]() {
    buttons->button(QDialogButtonBox::Ok)
        ->setEnabled(!m_table->selectedItems().isEmpty());
  });
  connect(m_table, &QTableWidget::cellDoubleClicked, this, &QDialog::accept);

  connect(&m_discovery, &Valeronoi::robot::MdnsDiscovery::robotDiscovered, this,
          &MdnsDiscoveryDialog::slot_robot_discovered);

  m_discovery.startDiscovery();
}

QString MdnsDiscoveryDialog::selectedUrl() const {
  const auto selected = m_table->selectedItems();
  if (selected.isEmpty()) return {};
  return m_table->item(selected.first()->row(), COL_NAME)
      ->data(Qt::UserRole)
      .toString();
}

void MdnsDiscoveryDialog::slot_robot_discovered(
    const Valeronoi::robot::DiscoveredRobot& robot) {
  const QString url = robot.url();
  for (int i = 0; i < m_table->rowCount(); ++i) {
    if (m_table->item(i, COL_NAME)->data(Qt::UserRole).toString() == url)
      return;
  }
  const int row = m_table->rowCount();
  m_table->insertRow(row);

  auto* nameItem = new QTableWidgetItem(robot.name);
  nameItem->setData(Qt::UserRole, url);
  m_table->setItem(row, COL_NAME, nameItem);
  m_table->setItem(row, COL_ID, new QTableWidgetItem(robot.id));
  m_table->setItem(row, COL_URL, new QTableWidgetItem(robot.url()));
  m_table->setItem(row, COL_MANUFACTURER,
                   new QTableWidgetItem(robot.manufacturer));
  m_table->setItem(row, COL_MODEL, new QTableWidgetItem(robot.model));
  m_table->setItem(row, COL_VERSION, new QTableWidgetItem(robot.version));
}
}  // namespace Valeronoi::gui::dialog
