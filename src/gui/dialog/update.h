#ifndef VALERONOI_GUI_DIALOG_UPDATE_H
#define VALERONOI_GUI_DIALOG_UPDATE_H

#include <QDialog>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
class UpdateDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {

class UpdateDialog : public QDialog {
  Q_OBJECT
 public:
  explicit UpdateDialog(QWidget *parent = nullptr);

  ~UpdateDialog() override;

  void check_update(bool silent = false, QWidget *error_parent = nullptr);

 private:
  Ui::UpdateDialog *ui;

  QUrl m_goto_url;

  QNetworkAccessManager m_qnam;
};

}  // namespace Valeronoi::gui::dialog

#endif
