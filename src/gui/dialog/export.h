#ifndef VALERONOI_GUI_DIALOG_EXPORT_H
#define VALERONOI_GUI_DIALOG_EXPORT_H

#include <QDialog>
#include <QSettings>
#include <QSize>
#include <QtWidgets>

#include "../../robot/robot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ExportDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {
class ExportDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ExportDialog(QWidget *parent = nullptr);

  ~ExportDialog() override;

  void set_size(const QSize &size);

  [[nodiscard]] QSize get_size() const;

  [[nodiscard]] bool get_transparent() const;

 private:
  Ui::ExportDialog *ui;
  double m_ratio;
};
}  // namespace Valeronoi::gui::dialog

#endif
