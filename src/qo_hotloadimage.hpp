#ifndef QO_HOTLOADIMAGE_HPP
#define QO_HOTLOADIMAGE_HPP

#include <atomic>
#include <mutex>
#include <tuple>
#include <thread>

#include <QObject>
#include <QImage>
#include <QList>
#include <QFileInfo>
#include <QThread>

typedef std::tuple<QImage, QFileInfo, bool> QFileImage;

class QImageLoadThreadPool : public QObject{
    Q_OBJECT
public:
    QImageLoadThreadPool(QObject *parent = 0) : QObject(parent) {
        QObject::connect(this, SIGNAL(loadComplete(QString,QImage)), this, SLOT(internalThreadEnd(QString,QImage)));
    }
    void takeControl() {controlExternal.lock();}
    void releaseControl() {controlExternal.unlock();}
    bool load(QString);
signals:
    void loadSuccess(QString, QImage);
    void loadFailed(QString);
    void loadComplete(QString, QImage);
private slots:
    void internalThreadEnd(QString, QImage);
private:
    void internalThreadRun(QString);
    void internalJoinThread(QString);
    std::thread *testThread = nullptr;
    std::mutex controlExternal;
    std::mutex controlInternal;
};

class QHotLoadImageBay : public QObject {
    Q_OBJECT
public:
    QHotLoadImageBay();
    void timerEvent(QTimerEvent*);
    void add(QFileInfo);
    QImage current();
    QImage next();
    QImage previous();
signals:
    void activeLoaded(QImage);
    void activeFailed();
private slots:
    void handleSuccess(QString, QImage);
    void handleFailure(QString);
private:
    QList<QFileImage> imgList;
    int internalGetNextIndex(int jump = 0) {
        if (imgList.length() > 0) {
            int nindex = index + (1 + jump);
            while (nindex >= imgList.length()) nindex -= imgList.length();
            return nindex;
        } else {
            return 0;
        }
    }
    int internalGetPreviousIndex(int jump = 0) {
        if (imgList.length() > 0) {
            int nindex = index - (1 + jump);
            while (nindex < 0) nindex += imgList.length();
            return nindex;
        } else {
            return 0;
        }
    }
    void internalNext() {index = internalGetNextIndex();}
    void internalPrevious() {index = internalGetPreviousIndex();}
    void internalSettleIndex() {internalNext(); internalPrevious();}
    int index = 0;
    QImageLoadThreadPool qiltp;
};

#endif // QO_HOTLOADIMAGE_HPP
