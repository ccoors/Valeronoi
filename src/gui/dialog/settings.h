#ifndef VALERONOI_GUI_DIALOG_SETTINGS_H
#define VALERONOI_GUI_DIALOG_SETTINGS_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {
class SettingsDialog : public QDialog {
  Q_OBJECT
 public:
  explicit SettingsDialog(QWidget *parent = nullptr);

  ~SettingsDialog() override;

  [[nodiscard]] bool should_auto_check_for_updates(QWidget *parent = nullptr);

 private:
  Ui::SettingsDialog *ui;
};
}  // namespace Valeronoi::gui::dialog

#endif
