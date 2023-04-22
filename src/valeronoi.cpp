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
#include "valeronoi.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QPainter>
#include <QSvgGenerator>
#include <QTextStream>

#include "config.h"

namespace Valeronoi {

#include "ui_valeronoi.h"

ValeronoiWindow::ValeronoiWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ValeronoiWindow),
      m_export_dialog{this},
      m_robot_config_dialog{this},
      m_settings_dialog{this},
      m_about_dialog{this},
      m_update_dialog{this},
      m_log_dialog{this},
      m_modified{false},
      m_current_file{""} {
  Valeronoi::util::LogHelper::instance().set_log_dialog(&m_log_dialog);
  qInfo().nospace() << "Starting Valeronoi " << VALERONOI_VERSION << " ("
                    << VALERONOI_GIT_COMMIT << ")";

  qDebug() << "Reading application settings";
  QSettings settings;
  const auto start_count = settings.value("app/startCount", 0).toInt();
  settings.setValue("app/startCount", start_count + 1);

  qDebug() << "Setting up UI";
  ui->setupUi(this);
  auto verticalLayout = new QVBoxLayout(ui->displayFrame);
  m_display_widget = new Valeronoi::gui::widget::DisplayWidget(
      m_robot_map, m_wifi_measurements, ui->displayFrame);
  verticalLayout->addWidget(m_display_widget);
  slot_end_recording();

#if __APPLE__
  ui->recordingStatsGroup->setFlat(true);
#endif

  m_wifi_measurements.set_map(m_robot_map);

  qDebug() << "Connecting signals/slots";
  connect_actions();
  connect_robot_signals();

  connect(
      &m_wifi_measurements,
      &Valeronoi::state::Measurements::signal_measurements_updated, this,
      [=]() {
        set_modified(true);
        m_display_widget->slot_measurements_updated();
        const auto stats = m_wifi_measurements.get_statistics();
        ui->labelMeasurements->setText(QString::number(stats.measurements));
        ui->labelUniquePlaces->setText(QString::number(stats.unique_places));
        if (stats.measurements > 0) {
          ui->labelWeakestSignal->setText(
              QString::number(stats.weakest).append(" dBm"));
          ui->labelStrongestSignal->setText(
              QString::number(stats.strongest).append(" dBm"));
        } else {
          ui->labelWeakestSignal->setText("-∞ dBm");
          ui->labelStrongestSignal->setText("-∞ dBm");
        }
      });

  connect(ui->connectToggle, &QPushButton::clicked, this, [=]() {
    if (m_robot.is_connecting() || m_robot.is_connected()) {
      m_robot.slot_disconnect();
    } else {
      m_connection_configuration.read_settings();
      if (!m_connection_configuration.is_valid()) {
        const auto clicked_button = QMessageBox::question(
            parent, tr("Configure robot"),
            tr("Robot Configuration is invalid or missing. Do you want to "
               "configure your robot?"));
        if (clicked_button == QMessageBox::StandardButton::Yes) {
          m_robot_config_dialog.load_settings();
          m_robot_config_dialog.show();
          m_robot_config_dialog.raise();
        }
        return;
      }
      m_robot.set_connection_configuration(m_connection_configuration);
      m_robot.slot_connect();
    }
  });
  connect(ui->recordingToggle, &QPushButton::clicked, this,
          &ValeronoiWindow::slot_toggle_recording);

  load_colormaps();
  connect_display_widget();

  update_title();
  if (m_settings_dialog.should_auto_check_for_updates(this)) {
    m_update_dialog.check_update(true, this);
  }
  ui->statusBar->showMessage(tr("Ready"));
}

ValeronoiWindow::~ValeronoiWindow() { delete ui; }

void ValeronoiWindow::closeEvent(QCloseEvent *event) {
  if (maybe_save()) {
    event->accept();
  } else {
    event->ignore();
  }
}

void ValeronoiWindow::newFile() {
  if (maybe_save()) {
    m_robot.slot_disconnect();
    m_current_file.clear();
    m_robot_map.reset();
    m_wifi_measurements.reset();
    set_modified(false);
    update_title();
  }
}

void ValeronoiWindow::update_title() {
  QString title = "Valeronoi " VALERONOI_VERSION;
  if (m_current_file.isEmpty()) {
    title.append(" - Untitled.").append(VALERONOI_FILE_EXTENSION);
  } else {
    QFileInfo info{m_current_file};
    title.append(" - ").append(info.fileName());
  }

  if (m_modified) {
    title.append("*");
  }

  setWindowTitle(title);
}

bool ValeronoiWindow::maybe_save() {
  if (!m_modified) {
    return true;
  }

  const auto ret = QMessageBox::warning(
      this, "Valeronoi",
      tr("The file has been modified.\n"
         "Do you want to save your changes?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  switch (ret) {
    case QMessageBox::Save:
      saveFile();
      return !m_modified;
    case QMessageBox::Cancel:
      return false;
    default:
      break;
  }
  return true;
}

bool ValeronoiWindow::save() {
  QFile file(m_current_file);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  auto object = QJsonObject();
  object.insert("map", m_robot_map.get_map_json());
  object.insert("measurements", m_wifi_measurements.get_json());
  object.insert("version", FILE_FORMAT_VERSION);
  auto data = QJsonDocument(object);
  file.write(data.toJson(QJsonDocument::Compact));
  file.close();
  set_modified(false);
  return true;
}

void ValeronoiWindow::openFile() {
  if (maybe_save()) {
    m_robot.slot_disconnect();
    QString filter = get_open_save_filter();
    const auto dir = get_open_save_dir();
    QString file_name = QFileDialog::getOpenFileName(
        this, tr("Select file to load"), dir, filter);

    if (!file_name.isEmpty()) {
      load_file(file_name);
    }
  }
}

QString ValeronoiWindow::get_open_save_filter() const {
  QString filter = tr("Valeronoi files")
                       .append(" (*.")
                       .append(VALERONOI_FILE_EXTENSION)
                       .append(")");
  return filter;
}

void ValeronoiWindow::set_modified(bool is_modified) {
  if (m_modified == is_modified) {
    return;
  }
  m_modified = is_modified;
  update_title();
}

void ValeronoiWindow::saveFile() {
  if (m_current_file.isEmpty()) {
    saveAsFile();
    return;
  }
  if (!save()) {
    QMessageBox::critical(this, "Valeronoi", tr("The file could not be saved"));
  } else {
    set_modified(false);
    update_title();
  }
}

void ValeronoiWindow::saveAsFile() {
  QString filter = get_open_save_filter();
  const auto dir = get_open_save_dir();
  QString file_name = QFileDialog::getSaveFileName(
      this, tr("Select file to save as"), dir, filter);
  if (!file_name.isEmpty()) {
    if (!file_name.endsWith(VALERONOI_FILE_EXTENSION)) {
      file_name.append(".").append(VALERONOI_FILE_EXTENSION);
    }
    QFileInfo info{file_name};
    set_open_save_dir(info.absoluteDir().absolutePath());
    m_current_file = file_name;
    saveFile();
  }
}

QString ValeronoiWindow::get_open_save_dir() const {
  QSettings settings;
  return settings.value("app/lastOpenSavePath", QDir::homePath()).toString();
}

void ValeronoiWindow::set_open_save_dir(const QString &dir) {
  QSettings settings;
  settings.setValue("app/lastOpenSavePath", dir);
}

void ValeronoiWindow::load_colormaps() {
  qDebug() << "Loading colormaps";
  QSettings settings;
  QString selectedMap = settings.value("display/colorMap", "").toString();
  QFile loadFile{":/res/colormaps.json"};

  if (!loadFile.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(this, "Valeronoio", tr("Could not load colormaps"));
    return;
  }

  ui->displayColorMap->clear();

  const auto raw_data = loadFile.readAll();
  loadFile.close();
  auto colormap_json = QJsonDocument::fromJson(raw_data).array();

  int map_index_to_set = 0;

  for (const auto &&json : colormap_json) {
    auto colormap = json.toObject();
    auto name = colormap["name"].toString();
    auto colors = colormap["colors"].toArray();
    std::vector<Valeronoi::util::RGBColorMapColors> color_map_colors;
    for (const auto c : colors) {
      const auto json_colors = c.toArray();
      color_map_colors.push_back(Valeronoi::util::RGBColorMapColors{
          json_colors[0].toDouble(), json_colors[1].toDouble(),
          json_colors[2].toDouble()});
    }
    if (name == selectedMap) {
      map_index_to_set = m_color_maps.size();
    }
    ui->displayColorMap->addItem(name);
    m_color_maps.append(Valeronoi::util::RGBColorMap(name.toStdString().c_str(),
                                                     color_map_colors));
  }

  ui->displayColorMap->setCurrentIndex(map_index_to_set);
  m_display_widget->set_color_map(&m_color_maps[map_index_to_set]);
}

bool ValeronoiWindow::load_file(const QString &path) {
  QFileInfo info{path};
  if (!info.exists()) {
    QMessageBox::warning(nullptr, "Error", tr("File does not exist"));
    return false;
  }
  // If load file here
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(nullptr, "Error", tr("Could not open file"));
    return false;
  }
  QJsonParseError error;
  auto json_document = QJsonDocument::fromJson(file.readAll(), &error);
  file.close();
  if (json_document.isNull() || json_document.isEmpty()) {
    QMessageBox::warning(
        nullptr, "Error",
        tr("Error parsing file:\n%1").arg(error.errorString()));
    return false;
  }
  if (json_document.object()["version"].toInt() != FILE_FORMAT_VERSION) {
    QMessageBox::warning(nullptr, "Error",
                         tr("File is incompatible with this version"));
    return false;
  }
  if (!json_document.object()["map"].isObject() ||
      !json_document.object()["measurements"].isArray()) {
    QMessageBox::warning(nullptr, "Error", tr("File is corrupted"));
    return false;
  }
  m_robot_map.update_map_json(json_document.object()["map"].toObject());
  m_wifi_measurements.set_json(
      json_document.object()["measurements"].toArray());

  set_open_save_dir(info.absoluteDir().absolutePath());
  m_current_file = info.absoluteFilePath();
  set_modified(false);
  update_title();
  return true;
}

void ValeronoiWindow::slot_begin_recording() {
  if (!m_robot.is_connected()) {
    return;
  }
  m_recording = true;
  ui->recordingToggle->setText(tr("End recording"));
}

void ValeronoiWindow::slot_end_recording() {
  m_recording = false;
  ui->recordingToggle->setText(tr("Begin recording"));
}

void ValeronoiWindow::slot_toggle_recording() {
  if (m_recording) {
    slot_end_recording();
  } else {
    slot_begin_recording();
  }
}

void ValeronoiWindow::update_display_settings() {
  ui->displayEntities->setChecked(m_display_widget->get_entities_enabled());
  ui->useOpenGL->setChecked(m_display_widget->get_opengl());
  ui->antialiasing->setChecked(m_display_widget->get_antialiasing());
  ui->displayRestrictFloor->setChecked(m_display_widget->get_restrict_floor());
  ui->displayRestrictPath->setChecked(m_display_widget->get_restrict_path());
  ui->simplifySlider->setValue(m_display_widget->get_simplify());
}

void ValeronoiWindow::connect_display_widget() {
  connect(ui->setBackgroundColor, &QPushButton::clicked, this, [=]() {
    auto color =
        QColorDialog::getColor(m_display_widget->get_background_color());
    if (color.isValid()) {
      m_display_widget->set_background_color(color);
    }
  });

  connect(ui->setWallColor, &QPushButton::clicked, this, [=]() {
    auto color = QColorDialog::getColor(m_display_widget->get_wall_color());
    if (color.isValid()) {
      m_display_widget->set_wall_color(color);
    }
  });

  ui->drawFloor->setChecked(m_display_widget->get_floor_enabled());
  connect(ui->drawFloor, &QCheckBox::stateChanged, this, [=]() {
    m_display_widget->set_floor(ui->drawFloor->isChecked(),
                                m_display_widget->get_floor_color());
  });
  connect(ui->setFloorColor, &QPushButton::clicked, this, [=]() {
    auto color = QColorDialog::getColor(m_display_widget->get_floor_color());
    if (color.isValid()) {
      m_display_widget->set_floor(m_display_widget->get_floor_enabled(), color);
    }
  });
  connect(ui->displayEntities, &QCheckBox::stateChanged, this, [=]() {
    m_display_widget->set_entities(ui->displayEntities->isChecked());
  });

  connect(&m_robot_map, &Valeronoi::state::RobotMap::signal_map_updated,
          m_display_widget,
          &Valeronoi::gui::widget::DisplayWidget::slot_map_updated);

  connect(ui->displayColorMap,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [=](int index) {
            m_display_widget->set_color_map(&m_color_maps[index]);
          });
  connect(ui->displayMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
          m_display_widget,
          &Valeronoi::gui::widget::DisplayWidget::slot_set_display_mode);
  connect(m_display_widget,
          &Valeronoi::gui::widget::DisplayWidget::signal_relocate, &m_robot,
          &Valeronoi::robot::Robot::slot_relocate);
  connect(ui->useOpenGL, &QCheckBox::stateChanged, this,
          [=]() { m_display_widget->set_opengl(ui->useOpenGL->isChecked()); });
  connect(ui->antialiasing, &QCheckBox::stateChanged, this, [=]() {
    m_display_widget->set_antialiasing(ui->antialiasing->isChecked());
  });
  connect(ui->displayRestrictFloor, &QCheckBox::stateChanged, this, [=]() {
    m_display_widget->set_restrict_floor(ui->displayRestrictFloor->isChecked());
  });
  connect(ui->displayRestrictPath, &QCheckBox::stateChanged, this, [=]() {
    m_display_widget->set_restrict_path(ui->displayRestrictPath->isChecked());
  });
  connect(ui->simplifySlider, &QSlider::valueChanged, m_display_widget,
          &Valeronoi::gui::widget::DisplayWidget::slot_set_simplify);

#ifdef QT_NO_OPENGL
  ui->useOpenGL->setChecked(false);
  ui->useOpenGL->setEnabled(false);
#endif

  update_display_settings();
}

void ValeronoiWindow::connect_actions() {
  connect(ui->actionNew, &QAction::triggered, this, &ValeronoiWindow::newFile);
  connect(ui->actionOpen, &QAction::triggered, this,
          &ValeronoiWindow::openFile);
  connect(ui->actionSave, &QAction::triggered, this,
          &ValeronoiWindow::saveFile);
  connect(ui->actionSaveAs, &QAction::triggered, this,
          &ValeronoiWindow::saveAsFile);
  connect(ui->actionExportAsImage, &QAction::triggered, this, [=]() {
    auto size = m_display_widget->sceneRect().size().toSize();
    size.scale(4000, 4000, Qt::KeepAspectRatio);
    m_export_dialog.set_size(size);
    m_export_dialog.show();
  });
  connect(&m_export_dialog, &QDialog::accepted, this, [=]() {
    QFileDialog file_dialog(this);
    file_dialog.setAcceptMode(QFileDialog::AcceptSave);
    file_dialog.setDirectory(get_open_save_dir());
    QStringList mime_types;
    const auto supported_mime_types = QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeType : supported_mime_types) {
      auto name = QLatin1String(mimeType);
      if (name != "image/jpeg" && name != "image/jp2") {
        mime_types.append(name);
      }
    }
    file_dialog.setMimeTypeFilters(mime_types);
    const int png_index = mime_types.indexOf("image/png");
    if (png_index >= 0) {
      file_dialog.selectMimeTypeFilter(mime_types.at(png_index));
      file_dialog.setDefaultSuffix("png");
    }
    if (file_dialog.exec() == QDialog::Accepted) {
      const auto file_name = file_dialog.selectedFiles().constFirst();
      const QSize image_size = m_export_dialog.get_size();
      QImage image(image_size, QImage::Format_ARGB32);
      if (m_export_dialog.get_transparent()) {
        image.fill(Qt::transparent);
      } else {
        image.fill(m_display_widget->get_background_color());
      }
      QPainter painter;
      painter.setRenderHints(QPainter::Antialiasing |
                             QPainter::TextAntialiasing);
      painter.begin(&image);
      if (m_export_dialog.get_transparent()) {
        m_display_widget->scene()->setBackgroundBrush(Qt::transparent);
      }
      m_display_widget->scene()->render(&painter,
                                        QRectF(QPointF(), QSizeF(image_size)));
      m_display_widget->set_background_color(
          m_display_widget->get_background_color());
      painter.end();

      if (!image.save(file_name)) {
        QMessageBox::warning(
            this, "Valeronoi",
            tr("Could not write image file %1").arg(file_name));
      }
    }
  });
  connect(ui->actionExportAsSVG, &QAction::triggered, this, [=]() {
    QMessageBox::information(
        this, "Valeronoi",
        tr("Please note that the SVG export is experimental and will generate "
           "geometry that is out of bounds."));
    QString export_path = QFileDialog::getSaveFileName(
        this, tr("Save SVG"), get_open_save_dir(), tr("SVG files (*.svg)"));
    if (export_path.isEmpty()) {
      return;
    }
    if (!export_path.endsWith(".svg")) {
      export_path.append(".svg");
    }
    const auto scene_size = m_display_widget->size();
    QSvgGenerator generator;
    generator.setFileName(export_path);
    generator.setSize(scene_size);
    generator.setViewBox(QRect(0, 0, scene_size.width(), scene_size.height()));
    generator.setTitle(tr("SVG generated by Valeronoi"));
    generator.setDescription(tr("A WiFi Heatmap generated by Valeronoi"));
    QPainter painter;
    painter.begin(&generator);
    m_display_widget->scene()->render(&painter);
    painter.end();
  });
  connect(ui->actionSettings, &QAction::triggered, this, [=]() {
    m_settings_dialog.show();
    m_settings_dialog.raise();
  });
  connect(ui->actionQuit, &QAction::triggered, this,
          [=]() { QCoreApplication::quit(); });
  connect(ui->actionCheckForUpdates, &QAction::triggered, this,
          [=]() { m_update_dialog.check_update(false, this); });
  connect(ui->actionLog, &QAction::triggered, this,
          [=]() { m_log_dialog.show(); });
  connect(ui->actionAbout, &QAction::triggered, this,
          [=]() { m_about_dialog.show(); });
  connect(ui->actionRobotManager, &QAction::triggered, this, [=]() {
    m_robot_config_dialog.load_settings();
    m_robot_config_dialog.show();
    m_robot_config_dialog.raise();
  });
}

void ValeronoiWindow::connect_robot_signals() {
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connecting, this, [=]() {
    ui->actionRobotManager->setEnabled(false);
    ui->connectToggle->setText(tr("Connecting"));
  });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connected, this, [=]() {
    ui->actionRobotManager->setEnabled(false);
    ui->recordingToggle->setEnabled(true);
    ui->connectToggle->setText(tr("Disconnect"));

    ui->buttonClean->setEnabled(true);
    ui->buttonHome->setEnabled(true);
    ui->buttonPause->setEnabled(true);
    ui->buttonStop->setEnabled(true);

    ui->buttonLocate->setEnabled(
        m_robot.get_information()->m_capabilities.contains("LocateCapability"));
  });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connection_ended, this,
          [=]() {
            slot_end_recording();
            ui->actionRobotManager->setEnabled(true);
            ui->recordingToggle->setEnabled(false);
            ui->connectToggle->setText(tr("Connect"));
            ui->buttonClean->setEnabled(false);
            ui->buttonHome->setEnabled(false);
            ui->buttonPause->setEnabled(false);
            ui->buttonStop->setEnabled(false);
            ui->buttonLocate->setEnabled(false);
          });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_connection_error, this,
          [=]() {
            const auto error = m_robot.get_error();
            QMessageBox::warning(this, tr("Error"),
                                 tr("Connection error: %1").arg(error));
          });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_wifi_updated, this,
          [=](double value) {
            ui->labelLatestSignal->setText(
                tr("%1 dBm").arg(static_cast<int>(value)));
            if (m_recording) {
              m_wifi_measurements.slot_add_measurement(value);
            }
          });
  connect(&m_robot, &Valeronoi::robot::Robot::signal_map_updated, this, [=]() {
    m_robot_map.update_map_json(m_robot.get_map_data());
    set_modified(true);
  });
  connect(ui->recordingInterval,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), &m_robot,
          &Valeronoi::robot::Robot::slot_subscribe_wifi);
  m_robot.slot_subscribe_wifi(ui->recordingInterval->value());

  connect(ui->buttonClean, &QPushButton::clicked, this, [=]() {
    m_robot.send_command(Valeronoi::robot::BASIC_COMMANDS::START);
  });
  connect(ui->buttonPause, &QPushButton::clicked, this, [=]() {
    m_robot.send_command(Valeronoi::robot::BASIC_COMMANDS::PAUSE);
  });
  connect(ui->buttonStop, &QPushButton::clicked, this, [=]() {
    m_robot.send_command(Valeronoi::robot::BASIC_COMMANDS::STOP);
  });
  connect(ui->buttonHome, &QPushButton::clicked, this, [=]() {
    m_robot.send_command(Valeronoi::robot::BASIC_COMMANDS::HOME);
  });
  connect(ui->buttonLocate, &QPushButton::clicked, this, [=]() {
    m_robot.send_command(Valeronoi::robot::BASIC_COMMANDS::LOCATE);
  });
}

}  // namespace Valeronoi
