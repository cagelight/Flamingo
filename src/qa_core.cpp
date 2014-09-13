#include "qa_core.hpp"
#include "qw_mainwindow.hpp"

FlamingoCore::FlamingoCore(int &argc, char **&argv): QApplication(argc, argv) {
    if (argc > 1) {
        QFileInfoList qfil;
        for (int i = 1; i < argc; i++) {
            qfil.append(QFileInfo(argv[i]));
        }
        windowMain = new FlamingoMainWindow(qfil);
    } else {
        windowMain = new FlamingoMainWindow(QFileInfoList({QFileInfo(QDir::currentPath())}));
    }
    windowMain->show();
    QObject::connect(windowMain, SIGNAL(closed()), this, SLOT(quit()));
}
