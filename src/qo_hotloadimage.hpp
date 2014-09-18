#ifndef QO_HOTLOADIMAGE_HPP
#define QO_HOTLOADIMAGE_HPP

#include <atomic>
#include <mutex>
#include <thread>

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>

class QHotLoadImage : public QObject {
    Q_OBJECT
public:
    QHotLoadImage(QFileInfo location, QObject *parent = 0);
    QImage getImage();
    QString getName() const {return info.fileName();}
    bool imgAvailable() const;
    bool hasFailed() const {return failState;}
public slots:
    void hotLoad();
    void hotTryUnload();
    void hotUnload();
signals:
    void LoadComplete(QImage);
    void LoadFailed();
private:
    void internalLoad();
    void joinRead();
    QFileInfo info;
    std::thread *readThread = nullptr;
    QImage *hotImage = nullptr;
    bool failState = false;
    QHotLoadImage(const QHotLoadImage&) = delete;
    QHotLoadImage& operator=(const QHotLoadImage&) = delete;
};

#endif // QO_HOTLOADIMAGE_HPP
