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
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QMetaType>
#include <QtWidgets>

#include "config.h"
#include "state/state.h"
#include "util/log_helper.h"
#include "valeronoi.h"

int main(int argc, char **argv) {
  qInstallMessageHandler(Valeronoi::util::log_handler);

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
  parser.process(app);

  Valeronoi::ValeronoiWindow valeronoi;
  if (!parser.positionalArguments().isEmpty()) {
    if (!valeronoi.load_file(parser.positionalArguments().value(0))) {
      return 1;
    }
  }
  valeronoi.show();

  return app.exec();
}
