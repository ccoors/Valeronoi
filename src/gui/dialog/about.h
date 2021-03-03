#ifndef VALERONOI_GUI_DIALOG_ABOUT_H
#define VALERONOI_GUI_DIALOG_ABOUT_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AboutDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {

class AboutDialog : public QDialog {
  Q_OBJECT
 public:
  explicit AboutDialog(QWidget *parent = nullptr);

  ~AboutDialog() override;

 private:
  Ui::AboutDialog *ui;
};

}  // namespace Valeronoi::gui::dialog

#endif
