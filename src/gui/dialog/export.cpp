#include "export.h"

#include "ui_export.h"

namespace Valeronoi::gui::dialog {
ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExportDialog) {
  ui->setupUi(this);

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
