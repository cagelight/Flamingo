#include "qo_hotloadimage.hpp"

#include <QDebug>

QImageLoadThreadPool::~QImageLoadThreadPool() {
    for (LoadThread &lt : workers) {
        if (lt.second->joinable()) lt.second->join();
        delete lt.second;
    }
}

bool QImageLoadThreadPool::load(QString path) {
    bool rv = false;
    controlInternal.lock();
    if (workers.length() < 3) {
        bool gtg = true;
        for (LoadThread &lt : workers) {
            if (lt.first == path) gtg = false;
        }
        if (gtg) {
            workers.append(LoadThread(path, new std::thread(std::bind(&QImageLoadThreadPool::internalThreadRun, this, path))));
            rv = true;
        }
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

void QImageLoadThreadPool::internalJoinThread(QString path) {
    controlInternal.lock();
    QMutableListIterator<LoadThread> qmli(workers);
    while(qmli.hasNext()) {
        LoadThread &lt = qmli.next();
        if (lt.first == path) {
            if (lt.second->joinable()) lt.second->join();
            delete lt.second;
            qmli.remove();
            break;
        }
    }
    controlInternal.unlock();
}

///--------------------------------------------///

QHotLoadImageBay::QHotLoadImageBay() : QObject(), lastDirection(D_NEUTRAL) {
    QObject::connect(&qiltp, SIGNAL(loadSuccess(QString,QImage)), this, SLOT(handleSuccess(QString,QImage)), Qt::DirectConnection);
    QObject::connect(&qiltp, SIGNAL(loadFailed(QString)), this, SLOT(handleFailure(QString)), Qt::DirectConnection);
    this->startTimer(50);
}

void QHotLoadImageBay::timerEvent(QTimerEvent *) {
    const int length = imgList.length();
    if (length > 0) {
        QList<int> nindicies;
        switch (length) {
        default:
        //    nindicies.push_front(internalGetPreviousIndex(1));
        //case 4:
        //    nindicies.push_front(internalGetNextIndex(1));
        //case 3:
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
                if (lastDirection == D_PREV) this->internalPrevious();
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
    this->lastDirection.store(D_NEXT);
    return current();
}

QImage QHotLoadImageBay::previous() {
    internalPrevious();
    this->lastDirection.store(D_PREV);
    return current();
}

QImage QHotLoadImageBay::skipTo(QFileInfo fi) {
    for (int i = 0; i < imgList.length(); i++) {
        const QFileInfo &lfi = std::get<1>(imgList.at(i));
        if (lfi == fi) {
            index = i;
            this->internalSettleIndex();
            return current();
        }
    }
    return current();
}

void QHotLoadImageBay::handleSuccess(QString comppath, QImage newimg) {
    for (int i = 0; i < imgList.length(); i++) {
        QImage &img = std::get<0>(imgList[i]);
        QFileInfo &info = std::get<1>(imgList[i]);
        bool &loaded = std::get<2>(imgList[i]);
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
            if (i < index) this->internalPrevious();
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
