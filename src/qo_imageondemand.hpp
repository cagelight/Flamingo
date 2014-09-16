#ifndef QO_IMAGEONDEMAND_HPP
#define QO_IMAGEONDEMAND_HPP

#include <thread>
#include <QImage>
#include <QImageReader>

class QPreloadableImage : public QObject {
    Q_OBJECT
public:
    QPreloadableImage(QString dir) : qir(dir) {}
    QPreloadableImage() : qir() {}
    ~QPreloadableImage() {
        if (readthread != nullptr) {
            if (readthread->joinable()) readthread->join();
            delete readthread;
        }
    }
    void beginRead();
    QImage getImage();
    bool isReadable() {return qir.canRead();}
private:
    void readImg();
    QImageReader qir;
    QImage img = QImage(0, 0);
    std::thread *readthread = nullptr;
    bool readComplete = false;
    QPreloadableImage(const QPreloadableImage&) = delete;
    QPreloadableImage& operator=(const QPreloadableImage&) = delete;
};

#endif // QO_IMAGEONDEMAND_HPP
