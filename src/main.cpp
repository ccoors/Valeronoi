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
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QMetaType>
#include <QtWidgets>

#include "cli/headless_recorder.h"
#include "config.h"
#include "state/state.h"
#include "util/log_helper.h"
#include "valeronoi.h"

int main(int argc, char** argv) {
  qInstallMessageHandler(Valeronoi::util::log_handler);

  // Check for --headless before creating QApplication.
  // Forces offscreen QPA platform so no display is required.
  bool headless_mode = false;
  for (int i = 1; i < argc; ++i) {
    if (QString(argv[i]) == "--headless") {
      headless_mode = true;
      qputenv("QT_QPA_PLATFORM", "offscreen");
      break;
    }
  }

  // If not headless, check if a display is available.
  // Prevents "Aborted (core dumped)" in containers or SSH sessions.
  if (!headless_mode) {
    const bool has_display = !qEnvironmentVariableIsEmpty("DISPLAY") ||
                             !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY") ||
                             !qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM");

    if (!has_display) {
      fprintf(stderr,
              "Error: No display available.\n"
              "Run with --headless for CLI mode, e.g.:\n"
              "  valeronoi --headless --url http://robot:80 "
              "--output out.vwm --duration 60\n"
              "\nOr set QT_QPA_PLATFORM=offscreen to suppress this.\n");
      return 1;
    }
  }

  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("ccoors");
  QCoreApplication::setOrganizationDomain("ccoors.de");
  QCoreApplication::setApplicationName("Valeronoi");
  QCoreApplication::setApplicationVersion(VALERONOI_VERSION);

  Q_INIT_RESOURCE(valeronoi);
  QFontDatabase::addApplicationFont(":/res/SourceCodePro-Regular.otf");
  QApplication::setWindowIcon(QIcon(":/res/valeronoi.png"));

  qRegisterMetaType<Valeronoi::state::DataSegments>();

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "A WiFi signal strength mapping companion for Valetudo");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "The file to open.");

  // Headless CLI options
  QCommandLineOption headlessOpt("headless", "Run without GUI (record mode)");
  QCommandLineOption outputOpt("output", "Output file path (e.g., 'run1.vwm')",
                               "file");
  QCommandLineOption durationOpt("duration", "Recording duration in seconds",
                                 "seconds");
  QCommandLineOption intervalOpt("interval", "Polling interval in seconds",
                                 "seconds", "5.0");
  QCommandLineOption urlOpt("url", "Robot Valetudo URL (e.g. http://robot:80)",
                            "url");
  QCommandLineOption authOpt("auth", "Enable HTTP basic auth");
  QCommandLineOption userOpt("user", "HTTP auth username", "username");
  QCommandLineOption passOpt("pass", "HTTP auth password", "password");
  QCommandLineOption commandOpt(
      "command",
      "Send robot command on connect (start, pause, stop, home, locate). "
      "Without --output, sends command and exits immediately.",
      "cmd");
  QCommandLineOption modeOpt(
      "mode", "Set operation mode before start (vacuum, mop, vacuum_and_mop)",
      "mode");
  QCommandLineOption returnHomeOpt(
      "return-home",
      "Send stop + home commands when duration elapsed or Ctrl+C is pressed");

  parser.addOption(headlessOpt);
  parser.addOption(outputOpt);
  parser.addOption(durationOpt);
  parser.addOption(intervalOpt);
  parser.addOption(urlOpt);
  parser.addOption(authOpt);
  parser.addOption(userOpt);
  parser.addOption(passOpt);
  parser.addOption(commandOpt);
  parser.addOption(modeOpt);
  parser.addOption(returnHomeOpt);

  parser.process(app);

  // ---- Headless CLI mode ----
  if (parser.isSet(headlessOpt)) {
    Valeronoi::cli::HeadlessRecorder recorder;
    recorder.set_output(parser.value(outputOpt));

    // Validate --duration: must be a positive integer (0 = unlimited)
    if (parser.isSet(durationOpt)) {
      bool ok = false;
      const int duration = parser.value(durationOpt).toInt(&ok);
      if (!ok || duration < 0) {
        fprintf(stderr,
                "Error: --duration must be a non-negative integer (seconds), "
                "got '%s'\n",
                qPrintable(parser.value(durationOpt)));
        return 1;
      }
      recorder.set_duration(duration);
    }

    // Validate --interval: must be a positive number > 0
    {
      bool ok = false;
      const double interval = parser.value(intervalOpt).toDouble(&ok);
      if (!ok || interval <= 0.0) {
        fprintf(stderr,
                "Error: --interval must be a positive number (seconds), "
                "got '%s'\n",
                qPrintable(parser.value(intervalOpt)));
        return 1;
      }
      recorder.set_interval(interval);
    }

    if (parser.isSet(urlOpt)) {
      recorder.set_url(parser.value(urlOpt));
    }
    if (parser.isSet(authOpt)) {
      recorder.set_auth(parser.value(userOpt), parser.value(passOpt));
    }
    if (parser.isSet(modeOpt)) {
      recorder.set_operation_mode(parser.value(modeOpt));
    }
    if (parser.isSet(commandOpt)) {
      recorder.set_command(parser.value(commandOpt));
    }
    if (parser.isSet(returnHomeOpt)) {
      recorder.set_return_home(true);
    }
    if (!parser.positionalArguments().isEmpty()) {
      recorder.load_file(parser.positionalArguments().value(0));
    }

    return recorder.run();
  }

  // ---- Normal GUI mode ----
  Valeronoi::ValeronoiWindow valeronoi;
  if (!parser.positionalArguments().isEmpty()) {
    if (!valeronoi.load_file(parser.positionalArguments().value(0))) {
      return 1;
    }
  }
  valeronoi.show();

  return app.exec();
}
