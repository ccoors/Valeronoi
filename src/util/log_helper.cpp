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

#include "log_helper.h"

#include <QDateTime>

namespace Valeronoi::util {

LogHelper &LogHelper::instance() {
  static LogHelper _instance;
  return _instance;
}

void LogHelper::log(const QString &msg) {
  if (!m_log_dialog) {
    m_buffer.append(msg);
  } else {
    m_log_dialog->log_message(msg);
  }
}

void LogHelper::set_log_dialog(Valeronoi::gui::dialog::LogDialog *log_dialog) {
  m_log_dialog = log_dialog;
  m_log_dialog->log_message(m_buffer);
  m_buffer.clear();
}

void log_handler(QtMsgType type, const QMessageLogContext &context,
                 const QString &msg) {
  (void)context;
  auto &helper = LogHelper::instance();
  QString level;
  switch (type) {
    case QtDebugMsg:
      level = "DEBUG";
      break;
    case QtWarningMsg:
      level = "WARN ";
      break;
    case QtCriticalMsg:
      level = "CRIT ";
      break;
    case QtFatalMsg:
      level = "FATAL";
      break;
    case QtInfoMsg:
      level = "INFO ";
      break;
  }
  const auto time = QDateTime::currentDateTime();
  helper.log("[ " + level + " ] " + time.toString("yyyy-MM-dd hh:mm:ss.zzz") +
             " " + msg);
}

}  // namespace Valeronoi::util
