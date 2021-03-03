#include <QCommandLineParser>
#include <QFontDatabase>
#include <QMetaType>
#include <QtWidgets>

#include "config.h"
#include "state/state.h"
#include "valeronoi.h"

int main(int argc, char **argv) {
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
