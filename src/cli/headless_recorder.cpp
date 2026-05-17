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
#include "headless_recorder.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTextStream>
#include <csignal>

// Global pointer for signal handler (graceful Ctrl+C)
static Valeronoi::cli::HeadlessRecorder *g_recorder = nullptr;

// NOTE: Not strictly async-signal-safe, but works reliably with Qt's event
// loop
static void signal_handler(int) {
  if (g_recorder) {
    QMetaObject::invokeMethod(g_recorder, "slot_duration_elapsed",
                              Qt::QueuedConnection);
  }
}

namespace Valeronoi::cli {

HeadlessRecorder::HeadlessRecorder(QObject *parent) : QObject(parent) {
  // Link measurements to robot map (provides robot position)
  m_measurements.set_map(m_robot_map);

  // Robot signals
  connect(&m_robot, &robot::Robot::signal_connected, this,
          &HeadlessRecorder::slot_connected);
  connect(&m_robot, &robot::Robot::signal_connection_error, this,
          &HeadlessRecorder::slot_connection_error);
  connect(&m_robot, &robot::Robot::signal_map_updated, this,
          &HeadlessRecorder::slot_map_updated);
  connect(&m_robot, &robot::Robot::signal_wifi_info_updated, this,
          &HeadlessRecorder::slot_wifi_info_updated);

  // Duration timer (single shot)
  m_duration_timer.setSingleShot(true);
  connect(&m_duration_timer, &QTimer::timeout, this,
          &HeadlessRecorder::slot_duration_elapsed);
}

void HeadlessRecorder::set_output(const QString &path) {
  m_output_path = path;
  // Append default file extension if none is present
  if (!m_output_path.isEmpty() && QFileInfo(m_output_path).suffix().isEmpty()) {
    m_output_path.append(".vwm");
  }
}

void HeadlessRecorder::set_duration(int seconds) {
  m_duration_seconds = seconds;
}

void HeadlessRecorder::set_interval(double seconds) {
  m_interval_seconds = seconds;
}

void HeadlessRecorder::set_url(const QString &url) { m_url = url; }

void HeadlessRecorder::set_auth(const QString &username,
                                const QString &password) {
  m_auth = true;
  m_username = username;
  m_password = password;
}

void HeadlessRecorder::set_command(const QString &command) {
  m_command = command.toLower();
}

void HeadlessRecorder::set_operation_mode(const QString &mode) {
  m_operation_mode = mode.toLower();
}

void HeadlessRecorder::set_return_home(bool enabled) {
  m_return_home = enabled;
}

void HeadlessRecorder::load_file(const QString &path) { m_load_path = path; }

int HeadlessRecorder::run() {
  QTextStream out(stdout);

  // If no output but a command is set, run in "command-only" mode
  if (m_output_path.isEmpty() && m_command.isEmpty()) {
    out << "Error: --output or --command is required in headless mode\n";
    out.flush();
    return 1;
  }

  // Load existing measurements if provided
  if (!m_load_path.isEmpty()) {
    QFile file(m_load_path);
    if (file.open(QIODevice::ReadOnly)) {
      auto doc = QJsonDocument::fromJson(file.readAll());
      auto obj = doc.object();
      if (obj.contains("measurements")) {
        m_measurements.set_json(obj.value("measurements").toArray());
      }
      if (obj.contains("map")) {
        m_robot_map.update_map_json(obj.value("map").toObject());
      }
      out << "Loaded existing data from " << m_load_path << "\n";
      out.flush();
    } else {
      out << "Warning: Could not open " << m_load_path << "\n";
      out.flush();
    }
  }

  // Build connection configuration from CLI args or QSettings
  robot::ConnectionConfiguration config;

  if (!m_url.isEmpty()) {
    // Use CLI parameters
    config.m_url = QUrl(m_url);
    config.m_auth = m_auth;
    config.m_username = m_username;
    config.m_password = m_password;
  } else {
    // Fall back to QSettings (configured via GUI)
    config.read_settings();
  }

  if (!config.is_valid()) {
    out << "Error: No valid robot connection configured.\n";
    out << "Use --url to specify the robot URL, e.g.:\n";
    out << "  valeronoi --headless --url http://robot:80 --output out.vwm\n";
    out.flush();
    return 1;
  }

  out << "Connecting to " << config.m_url.toString() << " ...\n";
  out.flush();

  m_robot.set_connection_configuration(config);
  m_robot.slot_connect();

  // Handle Ctrl+C / SIGTERM gracefully to save before exit
  g_recorder = this;
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  return QCoreApplication::exec();
}

void HeadlessRecorder::slot_connected() {
  if (m_exiting) return;

  QTextStream out(stdout);
  out << "Connected.";

  // Set operation mode before sending command
  // (e.g. "vacuum" instead of "vacuum_and_mop")
  if (!m_operation_mode.isEmpty()) {
    out << " Setting mode: " << m_operation_mode << ".";
    m_robot.set_operation_mode(m_operation_mode);
  }

  // Send robot command if specified
  if (!m_command.isEmpty()) {
    robot::BASIC_COMMANDS cmd;
    bool valid_cmd = true;

    if (m_command == "start" || m_command == "clean") {
      cmd = robot::BASIC_COMMANDS::START;
    } else if (m_command == "pause") {
      cmd = robot::BASIC_COMMANDS::PAUSE;
    } else if (m_command == "stop") {
      cmd = robot::BASIC_COMMANDS::STOP;
    } else if (m_command == "home" || m_command == "dock") {
      cmd = robot::BASIC_COMMANDS::HOME;
    } else if (m_command == "locate") {
      cmd = robot::BASIC_COMMANDS::LOCATE;
    } else {
      out << "\nWarning: Unknown command '" << m_command
          << "' (valid: start, pause, stop, home, locate)\n";
      valid_cmd = false;
    }

    if (valid_cmd) {
      out << " Sending command: " << m_command << ".";
      m_robot.send_command(cmd);
    }
  }

  // Command-only mode: no recording needed for non-start commands,
  // or when no output is specified.
  bool command_only =
      m_output_path.isEmpty() ||
      (!m_command.isEmpty() && m_command != "start" && m_command != "clean");

  if (command_only) {
    out << " Done.\n";
    out.flush();
    // Give the event loop time to actually send the HTTP requests
    // 2s delay: enough for local HTTP requests to complete before exit
    QTimer::singleShot(2000, this, [this]() {
      m_robot.slot_disconnect();
      fflush(stdout);
      QCoreApplication::exit(0);
    });
    return;
  }

  out << " Starting recording";
  if (m_duration_seconds > 0) {
    out << " for " << m_duration_seconds << "s";
    m_duration_timer.start(m_duration_seconds * 1000);
  }
  out << " (interval: " << m_interval_seconds << "s)\n";
  out.flush();

  // Start wifi polling with the configured interval
  m_robot.slot_subscribe_wifi(m_interval_seconds);
}

void HeadlessRecorder::slot_connection_error() {
  if (m_exiting) return;

  QTextStream err(stderr);
  err << "Connection error: " << m_robot.get_error() << "\n";
  err.flush();
  save_and_exit(1);
}

void HeadlessRecorder::slot_map_updated() {
  if (m_exiting) return;

  // Update local robot map from the robot's SSE map data
  const QString map_data = m_robot.get_map_data();
  if (!map_data.isEmpty()) {
    m_robot_map.update_map_json(map_data);
  }
}

void HeadlessRecorder::slot_wifi_info_updated(
    Valeronoi::robot::WifiInformation wifi_info) {
  if (m_exiting) return;

  QTextStream out(stdout);

  // Add measurement at current robot position
  m_measurements.slot_add_measurement(wifi_info.signal(),
                                      m_measurements.unknown_wifi_id);

  out << "  Measurement recorded (total: "
      << m_measurements.get_measurements().size() << ")\n";
  out.flush();
}

void HeadlessRecorder::slot_duration_elapsed() {
  if (m_exiting) return;
  m_exiting = true;

  QTextStream out(stdout);
  out << "\nDuration elapsed.";
  out.flush();

  // Stop all timers
  m_duration_timer.stop();
  m_robot.slot_stop_wifi_subscription();

  // Optionally stop cleaning and send robot home before saving
  if (m_return_home) {
    out << " Sending stop + home...\n";
    out.flush();
    m_robot.send_command(robot::BASIC_COMMANDS::STOP);
    m_robot.send_command(robot::BASIC_COMMANDS::HOME);
    // Give event loop time to send the HTTP requests
    QTimer::singleShot(3000, this, [this]() {
      QTextStream out(stdout);
      out << "Saving...\n";
      out.flush();
      m_robot.slot_disconnect();
      save_and_exit(0);
    });
  } else {
    out << " Saving...\n";
    out.flush();
    m_robot.slot_disconnect();
    save_and_exit(0);
  }
}

void HeadlessRecorder::save_and_exit(int code) {
  QTextStream out(stdout);

  if (!m_output_path.isEmpty() && !m_measurements.get_measurements().empty()) {
    QJsonObject root;
    root["measurements"] = m_measurements.get_json();
    root["map"] = m_robot_map.get_map_json();
    root["version"] = 1;

    QFile file(m_output_path);
    if (file.open(QIODevice::WriteOnly)) {
      file.write(QJsonDocument(root).toJson());
      out << "Saved " << m_measurements.get_measurements().size()
          << " measurements to " << m_output_path << "\n";
    } else {
      out << "Error: Could not write to " << m_output_path << "\n";
      code = 1;
    }
  } else if (m_measurements.get_measurements().empty()) {
    out << "No measurements recorded.\n";
  }

  out.flush();
  fflush(stdout);
  fflush(stderr);
  QCoreApplication::exit(code);
}

}  // namespace Valeronoi::cli
