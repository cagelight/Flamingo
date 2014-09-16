#include "qo_imageondemand.hpp"
#include <QDebug>

void QPreloadableImage::beginRead() {
    if (!readComplete && readthread == nullptr) {
        readthread = new std::thread(&QPreloadableImage::readImg, this);
    }
}

QImage QPreloadableImage::getImage() {
    if (readthread == nullptr) beginRead();
    if (!readComplete && readthread->joinable()) readthread->join();
    return img;
}

void QPreloadableImage::readImg() {
    if (qir.canRead()) img = qir.read(); else img = QImage(0, 0);
    readComplete = true;
}
