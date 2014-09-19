#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include <QWidget>
#include <QFileInfo>
class QGridLayout;
class QStatusBar;
class QFlamingoView;

class FlamingoMainWindow : public QWidget {
    Q_OBJECT
public: //Methods
    FlamingoMainWindow(QFileInfoList);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
signals:
    void closed();
private slots:
    void handleStatusUpdate(QString);
private: //Widgets
    FlamingoMainWindow();
    QGridLayout *layoutMain = nullptr;
    QFlamingoView *fview = nullptr;
    QStatusBar *istatbar = nullptr;
};

#endif // QWIDGET_MAINWINDOW_HPP
