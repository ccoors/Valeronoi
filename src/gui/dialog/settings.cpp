#include "settings.h"

#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

#include "ui_settings.h"

namespace Valeronoi::gui::dialog {
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);

  connect(ui->checkUpdates, &QCheckBox::stateChanged, this, [=]() {
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
