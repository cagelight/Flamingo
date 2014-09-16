#ifndef QO_FLAMINGOVIEWMGR_HPP
#define QO_FLAMINGOVIEWMGR_HPP

#include <memory>

#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QImageReader>

#include "qw_imgview.hpp"
#include "qo_imageondemand.hpp"

typedef QList<QDir> QDirList;

class QTrackedImage : public QObject {
    Q_OBJECT
public:
    QTrackedImage(const QFileInfo &fi, bool trackedDir) : info(new QFileInfo(fi)), iread(new QPreloadableImage(fi.canonicalFilePath())), trackedDir(trackedDir) {}
    //QTrackedImage() : info(), iread(), trackedDir(false) {}
    const QFileInfo* Info() {return info.get();}
    bool isTracked() {return trackedDir;}
    bool canRead() {return (this->Info()->exists() && iread->isReadable());}
    void beginRead() {return iread->beginRead();}
    QImage getImage() {return iread->getImage();}
private:
    std::shared_ptr<QFileInfo> info;
    std::shared_ptr<QPreloadableImage> iread;
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
    QTrackedImage *imgCur = nullptr;
    QTrackedImage *imgNext = nullptr;
    QTrackedImage *imgPrev = nullptr;
private: //Image File Management
    enum INFONAV {NEXT, NEXT_FILE, NEXT_DIR_FILE, RESET_DIR, NEXT_DIR, FORCE_FILE, FORCE_DIR, RESET};
    QTrackedImage* advanceImage(const QTrackedImage* begin, INFONAV method, bool reverse = false);
    void setImageCurrent();
};

#endif // QO_FLAMINGOVIEWMGR_HPP
