#include "qo_hotloadimage.hpp"

#include <QDebug>

bool QImageLoadThreadPool::load(QString path) {
    bool rv = false;
    controlInternal.lock();
    if (testThread == nullptr) {
        testThread = new std::thread(std::bind(&QImageLoadThreadPool::internalThreadRun, this, path));
        rv = true;
    }
    controlInternal.unlock();
    return rv;
}

void QImageLoadThreadPool::internalThreadEnd(QString path, QImage img) {
    this->internalJoinThread(path);
    controlExternal.lock();
    if (img.isNull()) emit loadFailed(path); else emit loadSuccess(path, img);
    controlExternal.unlock();
}

void QImageLoadThreadPool::internalThreadRun(QString path) {
    QImage img = QImage(path);
    controlExternal.lock();
    controlInternal.lock();
    emit loadComplete(path, img);
    controlInternal.unlock();
    controlExternal.unlock();
}

void QImageLoadThreadPool::internalJoinThread(QString) {
    controlInternal.lock();
    if (testThread != nullptr) {
        if (testThread->joinable()) testThread->join();
        delete testThread;
        testThread = nullptr;
    }
    controlInternal.unlock();
}

///--------------------------------------------///

QHotLoadImageBay::QHotLoadImageBay() : QObject() {
    QObject::connect(&qiltp, SIGNAL(loadSuccess(QString,QImage)), this, SLOT(handleSuccess(QString,QImage)));
    QObject::connect(&qiltp, SIGNAL(loadFailed(QString)), this, SLOT(handleFailure(QString)));
    this->startTimer(50);
}

void QHotLoadImageBay::timerEvent(QTimerEvent *) {
    const int length = imgList.length();
    if (length > 0) {
        QList<int> nindicies;
        switch (length) {
        default:
            nindicies.push_front(internalGetPreviousIndex(1));
        case 4:
            nindicies.push_front(internalGetNextIndex(1));
        case 3:
            nindicies.push_front(internalGetPreviousIndex());
        case 2:
            nindicies.push_front(internalGetNextIndex());
        case 1:
            nindicies.push_front(index);
        }
        for (int i : nindicies) {
            QFileInfo &info = std::get<1>(imgList[i]);
            bool &loaded = std::get<2>(imgList[i]);
            if (!loaded) {
                if (qiltp.load(info.canonicalFilePath())) {
                    qDebug() << info.canonicalFilePath() << "loading...";
                }
            }
        }
        for (int i = 0; i < imgList.length(); i++) {
            if (nindicies.contains(i)) continue;
            bool &loaded = std::get<2>(imgList[i]);
            if (loaded) {
                QImage &img = std::get<0>(imgList[i]);
                img = QImage();
                loaded = false;
            }
        }
    }
}

void QHotLoadImageBay::add(QFileInfo info) {
    this->imgList.append(std::make_tuple(QImage(), info, false));
}

QImage QHotLoadImageBay::current() {
    if (imgList.length() > 0) {
        QImage &img = std::get<0>(imgList[index]);
        QFileInfo &info = std::get<1>(imgList[index]);
        bool &loaded = std::get<2>(imgList[index]);;
        if (loaded) {
            if (img.isNull()) {
                imgList.removeAt(index);
                this->internalSettleIndex();
                return current();
            } else {
                emit activeStatusUpdate(info.fileName() + QString(" loaded."));
                return img;
            }
        } else {
            emit activeStatusUpdate(info.fileName() + QString(" not loaded. Worker threads will load shortly..."));
            return QImage(0, 0);
        }
    } else {
        return QImage(0, 0);
    }
}

QImage QHotLoadImageBay::next() {
    internalNext();
    return current();
}

QImage QHotLoadImageBay::previous() {
    internalPrevious();
    return current();
}

void QHotLoadImageBay::handleSuccess(QString comppath, QImage newimg) {
    for (int i = 0; i < imgList.length(); i++) {
        QImage &img = std::get<0>(imgList[i]);
        QFileInfo &info = std::get<1>(imgList[i]);;
        bool &loaded = std::get<2>(imgList[i]);;
        if (info.canonicalFilePath() == comppath) {
            img = newimg;
            loaded = true;
            QFileInfo &curInfo = std::get<1>(imgList[index]);;
            if (curInfo == info) {
                emit activeStatusUpdate(info.fileName() + QString(" loaded."));
                emit activeLoaded(img);
            }
            return;
        } else {
            continue;
        }
    }
}

void QHotLoadImageBay::handleFailure(QString comppath) {
    for (int i = 0; i < imgList.length(); i++) {
        QFileInfo &info = std::get<1>(imgList[i]);;
        if (info.canonicalFilePath() == comppath) {
            imgList.removeAt(i);
            this->internalSettleIndex();
            if (imgList.length() > 0) {
                QFileInfo &curInfo = std::get<1>(imgList[index]);
                if (curInfo.canonicalFilePath() == info.canonicalFilePath()) {
                    emit activeFailed();
                }
            }
            return;
        } else {
            continue;
        }
    }
}
