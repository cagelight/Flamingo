#include "qa_core.hpp"
#include "qw_mainwindow.hpp"

#include <QDir>

FlamingoCore::FlamingoCore(int &argc, char **&argv): QApplication(argc, argv) {
    if (argc > 1) {
        QFileInfoArgumentList qfil;
        QList<char> curOpt;
        for (int i = 1; i < argc; i++) {
            QString arg = QString(argv[i]);
            if (arg.startsWith('-') && arg.length() > 1) {
                arg.remove(0, 1);
                for (QChar O : arg) {
                    if (!curOpt.contains(O.toLatin1())) curOpt.append(O.toLatin1());
                }
            } else {
                QFileInfoArgument file = QFileInfoArgument(arg);
                for (char O : curOpt) {
                    switch (O) {
                    case 'R':
                        file.setRecursive(true);
                        break;
                    }
                }
                curOpt.clear();
                qfil.append(file);
            }
        }
        windowMain = new FlamingoMainWindow(qfil);
    } else {
        windowMain = new FlamingoMainWindow(QFileInfoArgumentList({QFileInfoArgument(QDir::currentPath(), false)}));
    }
    windowMain->show();
    QObject::connect(windowMain, SIGNAL(closed()), this, SLOT(quit()));
}
