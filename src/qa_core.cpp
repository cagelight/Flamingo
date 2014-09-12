#include "qa_core.hpp"
#include "qw_mainwindow.hpp"

FlamingoCore::FlamingoCore(int &argc, char **argv): QApplication(argc, argv) {
    windowMain = new FlamingoMainWindow();
    windowMain->show();
}
