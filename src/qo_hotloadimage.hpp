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


typedef std::pair<QString, std::thread*> LoadThread;

class QImageLoadThreadPool : public QObject{
    Q_OBJECT
public:
    enum PATH_STATUS {PATH_NULL, PATH_SUCCESS, PATH_FAILURE, PATH_INSUFFICIENT_MEMORY, PATH_ALREADY_LOADING, PATH_MAX_WORKERS};
    QImageLoadThreadPool(int maxBytes = 805306368, QObject *parent = 0) : QObject(parent), maxBytes(maxBytes) {
        QObject::connect(this, SIGNAL(loadComplete(QString,QImage)), this, SLOT(internalThreadEnd(QString,QImage)), Qt::QueuedConnection);
    }
    ~QImageLoadThreadPool();
    void takeControl() {controlExternal.lock();}
    void releaseControl() {controlExternal.unlock();}
    PATH_STATUS load(QString, int &bytesLoaded);
signals:
    void loadSuccess(QString, QImage);
    void loadFailed(QString);
    void loadComplete(QString, QImage);
private slots:
    void internalThreadEnd(QString, QImage);
private:
    int maxBytes;
    void internalThreadRun(QString);
    void internalJoinThread(QString);
    QList<LoadThread> workers;
    std::mutex controlExternal;
    std::mutex controlInternal;
};

typedef std::tuple<QImage, QFileInfo, bool> QFileImage;
typedef QImageLoadThreadPool QILTP;

class QHotLoadImageBay : public QObject {
    Q_OBJECT
public:
    QHotLoadImageBay();
    void timerEvent(QTimerEvent*);
    void add(QFileInfo);
    QImage current();
    QImage next();
    QImage previous();
    QImage random();
    QImage skipTo(QFileInfo);
signals:
    void activeLoaded(QImage);
    void activeFailed();
    void activeStatusUpdate(QString);
public slots:
    void Activate();
    void Deactivate();
private slots:
    void handleSuccess(QString, QImage);
    void handleFailure(QString);
private:
    enum Direction {D_PREV, D_NEXT, D_NEUTRAL};
    std::atomic<Direction> lastDirection;
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
    void internalRandom() {index = qrand() % imgList.length();}
    void internalSettleIndex() {internalNext(); internalPrevious();}
    void unload(int index);
    void remove(int index);
    int index = 0;
    QImageLoadThreadPool qiltp;
    bool activationState = false;
    int bytesLoaded = 0;
};

#endif // QO_HOTLOADIMAGE_HPP
