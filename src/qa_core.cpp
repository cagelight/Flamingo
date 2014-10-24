#include "qa_core.hpp"
#include "qw_mainwindow.hpp"
#include "qw_flamingoargmanager.hpp"

#include <QDir>

FlamingoCore::FlamingoCore(int &argc, char **&argv): QApplication(argc, argv) {
    if (argc > 1) {
        if (argc == 2 && QFlamingoLoadInformationData::fileIsFLID(QString(argv[1])))
                windowMain = new FlamingoMainWindow(QFlamingoLoadInformationData::load(QString(argv[1]), true));
        else {
            QFileInfoArgumentList qfil;
            QList<char> allOpt;
            QList<char> curOpt;
            for (int i = 1; i < argc; i++) {
                QString arg = QString(argv[i]);
                if (arg.startsWith('-') && arg.length() > 1) {
                    arg.remove(0, 1);
                    for (QChar O : arg) {
                        if (O.isUpper())
                        {if (!allOpt.contains(O.toLatin1())) allOpt.append(O.toLatin1());}
                        else
                        {if (!curOpt.contains(O.toLatin1())) curOpt.append(O.toLatin1());}
                    }
                } else {
                    QFileInfoArgument file = QFileInfoArgument(arg);
                    for (char O : allOpt) {
                        switch (O) {
                        case 'R':
                            file.setRecursive(true);
                            break;
                        }
                    }
                    for (char O : curOpt) {
                        switch (O) {
                        case 'r':
                            file.setRecursive(true);
                            break;
                        }
                    }
                    curOpt.clear();
                    qfil.append(file);
                }
            }
            windowMain = new FlamingoMainWindow(qfil);
        }
    } else {
        windowMain = new FlamingoMainWindow(QFileInfoArgumentList({QFileInfoArgument(QDir::currentPath(), false)}));
    };
    windowMain->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(windowMain, SIGNAL(closed()), this, SLOT(quit()));
}
