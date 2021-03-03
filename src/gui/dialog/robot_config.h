#ifndef VALERONOI_GUI_DIALOG_ROBOT_CONFIG_H
#define VALERONOI_GUI_DIALOG_ROBOT_CONFIG_H

#include <QDialog>
#include <QJsonDocument>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSettings>
#include <QUrl>
#include <QtWidgets>

#include "../../robot/robot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class RobotConfigDialog;
}
QT_END_NAMESPACE

namespace Valeronoi::gui::dialog {
class RobotConfigDialog : public QDialog {
  Q_OBJECT
 public:
  explicit RobotConfigDialog(QWidget *parent = nullptr);

  ~RobotConfigDialog() override;

  void load_settings();

 signals:
  void signal_config_changed();

 private slots:
  void slot_save();

  void slot_test_connection();

  void slot_end_test();

  void slot_robot_connected();

  void slot_robot_connection_failed();

 private:
  Ui::RobotConfigDialog *ui;
  QProgressDialog m_progress_dialog;
  bool m_test_cancelled{false};

  Valeronoi::robot::Robot m_robot;
};
}  // namespace Valeronoi::gui::dialog

#endif
