#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include "qfileinfoargument.hpp"
#include <QtWidgets>
#include <QFileInfo>
class QFlamingoView;

class FlamingoMainWindow : public QWidget {
    Q_OBJECT
public: //Methods
    FlamingoMainWindow(QFileInfoArgumentList);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void wheelEvent(QWheelEvent *);
signals:
    void closed();
private slots:
    void handleStatusUpdate(QString);
    void handleImageChange(QString);
    void startSlideshow();
    void stopSlideshow();
private: //Widgets
    FlamingoMainWindow();
    QGridLayout *layoutMain = new QGridLayout(this);
    QFlamingoView *fview = nullptr;
    QStatusBar *istatbar = new QStatusBar(this);
    //Slideshow
    QWidget *widgetSS = new QWidget(this);
    QHBoxLayout *layoutSS = new QHBoxLayout(this);
    QLabel *ssLabel = new QLabel("Slideshow: ", this);
    QSpinBox *ssSpin = new QSpinBox(this);
    QLabel *ssLabelSec = new QLabel("sec ", this);
    QCheckBox *ssShuffle = new QCheckBox("Shuffle", this);
    QPushButton *ssStart = new QPushButton("Start", this);
    QTimer *ssTimer = new QTimer(this);
};

#endif // QWIDGET_MAINWINDOW_HPP
