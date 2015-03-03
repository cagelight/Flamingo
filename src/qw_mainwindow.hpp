#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include "qfileinfoargument.hpp"
#include "qw_flamingoargmanager.hpp"
#include <QtWidgets>
#include <QFileInfo>

class QFlamingoView;
class QFlamingoArgManager;

class FlamingoMainWindow : public QMainWindow {
    Q_OBJECT
public: //Methods
    FlamingoMainWindow(QFlamingoLoadInformationData);
    virtual ~FlamingoMainWindow();
    void closeEvent(QCloseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent *);
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
    void randomb();
    void toggleHidden();
    void toggleFullscreen(bool);
    void toggleBorder(bool);
    void slideshowNext();
    void reloadArguments(QFileInfoArgumentList);
    void showViewLoadDialog();
private:
    QSettings appSettings {QSettings::UserScope, "Sensory Systems", "Flamingo", this};
private: //Widgets
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
    QAction * fileArgAction = nullptr;
    QAction * viewKeepFit = nullptr;
    QAction * viewKeepFitForce = nullptr;
    QAction * viewKeepExpanded = nullptr;
    QAction * viewKeepEquals = nullptr;
    QDialog * viewLoadKeepDialog = new QDialog(this);
    QRadioButton * viewLoadKeepFit = new QRadioButton("Fit (Don't Zoom)", viewLoadKeepDialog);
    QRadioButton * viewLoadKeepFitForce = new QRadioButton("Fit (Zoom To Fit)", viewLoadKeepDialog);
    QRadioButton * viewLoadKeepExpanded = new QRadioButton("Expand To Fill", viewLoadKeepDialog);
    QRadioButton * viewLoadKeepEquals = new QRadioButton("1:1 Center", viewLoadKeepDialog);
    //Arg
    QFlamingoArgManager * argManager = nullptr;
private: //Key Sequences
    static constexpr Qt::Key keyNavigationNext = Qt::Key_Right;
    static constexpr Qt::Key keyNavigationPrev = Qt::Key_Left;
    static constexpr Qt::Key keyNavigationRand = Qt::Key_Up;
    static constexpr Qt::Key keyNavigationRandB = Qt::Key_Down;
};

#endif // QWIDGET_MAINWINDOW_HPP
