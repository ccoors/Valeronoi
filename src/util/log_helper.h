/**
 * Valeronoi is an app for generating WiFi signal strength maps
 * Copyright (C) 2021 Christian Friedrich Coors <me@ccoors.de>
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
#ifndef VALERONOI_UTIL_LOG_HELPER_H
#define VALERONOI_UTIL_LOG_HELPER_H

#include <QApplication>
#include <QString>

#include "../gui/dialog/log.h"

namespace Valeronoi::util {

class LogHelper {
 public:
  static LogHelper &instance();

  LogHelper(const LogHelper &) = delete;

  LogHelper &operator=(const LogHelper &) = delete;

  void set_log_dialog(Valeronoi::gui::dialog::LogDialog *log_dialog);

  void log(const QString &msg);

 private:
  LogHelper() = default;

  QString m_buffer;
  Valeronoi::gui::dialog::LogDialog *m_log_dialog{nullptr};
};

void log_handler(QtMsgType type, const QMessageLogContext &context,
                 const QString &msg);

}  // namespace Valeronoi::util

#endif
