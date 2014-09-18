#include "qo_hotloadimage.hpp"

#include <QDebug>

QHotLoadImage::QHotLoadImage(QFileInfo location, QObject *parent) : QObject(parent), info(location) {

}

void QHotLoadImage::hotLoad() {
    if (hotImage == nullptr) {
        if (readThread == nullptr) {
            readThread = new std::thread(&QHotLoadImage::internalLoad, this);
        }
    }
}

void QHotLoadImage::hotTryUnload() {
    if (hotImage != nullptr) {
        this->hotUnload();
    }
}

void QHotLoadImage::hotUnload() {
    this->joinRead();
    if (hotImage != nullptr) {delete hotImage; hotImage = nullptr;}
}

QImage QHotLoadImage::getImage() {
    this->hotLoad();
    this->joinRead();
    return *hotImage;
}

bool QHotLoadImage::imgAvailable() const {
    return (hotImage != nullptr && !hotImage->isNull());
}

void QHotLoadImage::internalLoad() {
    if (hotImage != nullptr) delete hotImage;
    QImage *loadImg = new QImage(info.canonicalFilePath());
    hotImage = loadImg;
    if (hotImage->isNull()) {failState = true; emit LoadFailed();} else emit LoadComplete(*hotImage);
}

void QHotLoadImage::joinRead() {
    if (readThread != nullptr) {
        if (readThread->joinable()) readThread->join();
        delete readThread;
        readThread = nullptr;
    }
}
