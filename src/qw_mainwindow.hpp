#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include "qfileinfoargument.hpp"
#include <QtWidgets>
#include <QFileInfo>

class QFlamingoView;
class QFlamingoArgManager;

class FlamingoMainWindow : public QWidget {
    Q_OBJECT
public: //Methods
    FlamingoMainWindow(QFileInfoArgumentList);
    void closeEvent(QCloseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *);
signals:
    void closed();
private slots:
    void handleStatusUpdate(QString);
    void handleImageChange(QString);
    void startSlideshow();
    void stopSlideshow();
    void toggleSlideshow();
    void next();
    void previous();
    void random();
    void toggleHidden();
    void toggleFullscreen(bool);
    void toggleBorder(bool);
    void slideshowNext();
    void reloadArguments(QFileInfoArgumentList);
private: //Widgets
    FlamingoMainWindow();
    QGridLayout *layoutMain = new QGridLayout(this);
    QFlamingoView *fview = nullptr;
    QStatusBar *istatbar = new QStatusBar(this);
    //Menu
    QMenuBar * menu = new QMenuBar(this);
    QDialog * slideshowIntervalDialog = new QDialog(this);
    QSpinBox * slideshowIntervalSpinbox = new QSpinBox(slideshowIntervalDialog);
    QAction * slideshowStartAction = nullptr;
    QAction * slideRandAction = nullptr;
    QAction * mwindowBorder = nullptr;
    QTimer * slideshowTimer = new QTimer(this);
    //Arg
    QFlamingoArgManager * argManager = nullptr;
private: //Key Sequences
    static constexpr Qt::Key keyNavigationNext = Qt::Key_Right;
    static constexpr Qt::Key keyNavigationPrev = Qt::Key_Left;
    static constexpr Qt::Key keyNavigationRand = Qt::Key_Up;
};

#endif // QWIDGET_MAINWINDOW_HPP
