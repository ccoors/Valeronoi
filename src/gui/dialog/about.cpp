#include "about.h"

#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>

#include "config.h"
#include "ui_about.h"

namespace Valeronoi::gui::dialog {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);
  ui->version->setText(VALERONOI_VERSION);
  ui->gitCommit->setText(VALERONOI_GIT_COMMIT);

  connect(ui->aboutQt, &QPushButton::clicked, this,
          [=]() { QMessageBox::aboutQt(this); });
}

AboutDialog::~AboutDialog() { delete ui; }

}  // namespace Valeronoi::gui::dialog
