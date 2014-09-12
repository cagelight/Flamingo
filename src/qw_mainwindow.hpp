#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include <QWidget>
class QGridLayout;
class QImageView;

class FlamingoMainWindow : public QWidget {
    Q_OBJECT
public:
    FlamingoMainWindow();
private: //Widgets
    QGridLayout *layoutMain = nullptr;
    QImageView *iview = nullptr;
};

#endif // QWIDGET_MAINWINDOW_HPP
