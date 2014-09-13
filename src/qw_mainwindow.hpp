#ifndef QWIDGET_MAINWINDOW_HPP
#define QWIDGET_MAINWINDOW_HPP

#include <QWidget>
#include <QDir>
#include <QFileInfo>
class QGridLayout;
class QImageView;

typedef QList<QDir> QDirList;

class QTrackedFile : public QFileInfo {
public:
    QTrackedFile(const QFileInfo &fi, bool trackedDir) : QFileInfo(fi), trackedDir(trackedDir) {}
    QTrackedFile() : QFileInfo(), trackedDir(false) {}
    bool isTracked() {return trackedDir;}
private:
    bool trackedDir;
};

class FlamingoMainWindow : public QWidget {
    Q_OBJECT
public: //Variables
    const QImage *noimg;
public: //Methods
    FlamingoMainWindow(QFileInfoList);
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
signals:
    void closed();
public slots:
    void advanceImage();
private: //Image File Management
    enum INFONAV {NEXT, NEXT_FILE, NEXT_DIR_FILE, NEXT_DIR, FORCE_FIRST_FILE, FORCE_FIRST_DIR, RESET_BEGIN};
    //PREV, PREV_FILE, PREV_DIR_FILE, PREV_DIR, FORCE_LAST_FILE, FORCE_LAST_DIR, RESET_END
    void advanceImage(INFONAV);
    bool setCurrent(QTrackedFile);
private: //Variables
    QDirList imgDirs;
    QFileInfoList imgFiles;
    QTrackedFile imgCur;
private: //Widgets
    FlamingoMainWindow();
    QGridLayout *layoutMain = nullptr;
    QImageView *iview = nullptr;
};

#endif // QWIDGET_MAINWINDOW_HPP
