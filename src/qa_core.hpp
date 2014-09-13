#ifndef QAPPLICATION_MAIN_HPP
#define QAPPLICATION_MAIN_HPP

#include <QApplication>

class FlamingoMainWindow;

class FlamingoCore : public QApplication {
    Q_OBJECT
public:
    FlamingoCore(int &argc, char **&argv);
private: //Widgets
    FlamingoMainWindow *windowMain = nullptr;
};

#endif // QAPPLICATION_MAIN_HPP
