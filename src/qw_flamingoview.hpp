#ifndef QO_FLAMINGOVIEWMGR_HPP
#define QO_FLAMINGOVIEWMGR_HPP

#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include "qw_imgview.hpp"

typedef QList<QDir> QDirList;

class QTrackedFile : public QFileInfo {
public:
    QTrackedFile(const QFileInfo &fi, bool trackedDir) : QFileInfo(fi), trackedDir(trackedDir) {}
    QTrackedFile() : QFileInfo(), trackedDir(false) {}
    bool isTracked() {return trackedDir;}
private:
    bool trackedDir;
};

class QFlamingoView : public QImageView {
    Q_OBJECT
public: //Methods
    QFlamingoView(QFileInfoList fi, QWidget *parent = 0);
public slots:
    void Next();
    void Prev();
private: //Variables
    QDirList imgDirs;
    QFileInfoList imgFiles;
    QTrackedFile imgCur;
private: //Image File Management
    enum INFONAV {NEXT, NEXT_FILE, NEXT_DIR_FILE, RESET_DIR, NEXT_DIR, FORCE_FILE, FORCE_DIR, RESET};
    void advanceImage(INFONAV method, bool reverse = false);
    bool setCurrent(QTrackedFile);
    void setImageCurrent();
};

#endif // QO_FLAMINGOVIEWMGR_HPP
