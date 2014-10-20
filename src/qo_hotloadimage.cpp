#include "qo_hotloadimage.hpp"

#include <QDebug>
#include <QImageReader>
#include <ctime>

QImageLoadThreadPool::~QImageLoadThreadPool() {
    for (LoadThread &lt : workers) {
        if (lt.second->joinable()) lt.second->join();
        delete lt.second;
    }
}

QImageLoadThreadPool::PATH_STATUS QImageLoadThreadPool::load(QString path, int &bytesLoaded) {
    PATH_STATUS status = PATH_NULL;
    controlInternal.lock();
    if (workers.length() < 3) {
        bool gtg = true;
        for (LoadThread &lt : workers) {
            if (lt.first == path) gtg = false;
        }
        if (gtg) {
            QImageReader imgTest(path);
            QSize tSize = imgTest.size();
            if (tSize.isEmpty()) {
                status = PATH_FAILURE;
            } else {
                int bytesToLoad = tSize.width() * tSize.height() * 4;
                if (bytesLoaded + bytesToLoad > maxBytes) {
                    status = PATH_INSUFFICIENT_MEMORY;
                } else {
                    bytesLoaded += bytesToLoad;
                    workers.append(LoadThread(path, new std::thread(std::bind(&QImageLoadThreadPool::internalThreadRun, this, path))));
                    status = PATH_SUCCESS;
                }
            }
        } else {
            status = PATH_ALREADY_LOADING;
        }
    } else {
        status = PATH_MAX_WORKERS;
    }
    controlInternal.unlock();
    return status;
}

void QImageLoadThreadPool::joinAll() {
    for (LoadThread &lt : workers) {
        if (lt.second->joinable()) lt.second->join();
    }
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
    qsrand(time(0));
    QObject::connect(&qiltp, SIGNAL(loadSuccess(QString,QImage)), this, SLOT(handleSuccess(QString,QImage)), Qt::DirectConnection);
    QObject::connect(&qiltp, SIGNAL(loadFailed(QString)), this, SLOT(handleFailure(QString)), Qt::DirectConnection);
    this->startTimer(50);
}

void QHotLoadImageBay::timerEvent(QTimerEvent *) {
    if (activationState) {
        const int length = imgList.length();
        if (length > 0) {
            QList<int> nindicies;
            nindicies.prepend(this->internalGetPreviousIndex(1));
            nindicies.prepend(this->internalGetNextIndex(1));
            nindicies.prepend(this->internalGetPreviousIndex(0));
            nindicies.prepend(this->internalGetNextIndex(0));
            nindicies.prepend(index);
            QListIterator<int> iter(nindicies);
            for (int i = 0; i < imgList.length(); i++) {
                if (nindicies.contains(i)) continue;
                this->unload(i);
            }
            while (iter.hasNext()) {
                int i = iter.next();
                QFileInfo &info = std::get<1>(imgList[i]);
                bool &loaded = std::get<2>(imgList[i]);
                if (loaded) continue;
                QILTP::PATH_STATUS status;
                status = qiltp.load(info.canonicalFilePath(), bytesLoaded);
                switch (status) {
                case QILTP::PATH_NULL:
                    qDebug() << "Shouldn't see this message. QILTP::load() returned PATH_NULL.";
                    break;
                case QILTP::PATH_SUCCESS:
                    continue;
                case QILTP::PATH_FAILURE:
                    this->remove(i);
                    if (i < index) this->internalPrevious();
                    return;
                case QILTP::PATH_INSUFFICIENT_MEMORY:
                    iter.toBack();
                    while (iter.hasPrevious()) {
                        int iu = iter.previous();
                        if (i == iu) return ;else {
                            bool &loadedu = std::get<2>(imgList[iu]);
                            if (loadedu) this->unload(iu);
                            else continue;
                            return;
                        }
                    }
                    return;
                case QILTP::PATH_ALREADY_LOADING:
                    continue;
                case QILTP::PATH_MAX_WORKERS:
                    return;
                }
            }
        }
    }
}

void QHotLoadImageBay::add(QFileInfo info) {
    this->imgList.append(std::make_tuple(QImage(), info, false));
    if (imgList.length() == 1) emit imageChanged(std::get<1>(imgList.at(0)).fileName());
}

QImage QHotLoadImageBay::current() {
    if (imgList.length() > 0) {
        QImage &img = std::get<0>(imgList[index]);
        QFileInfo &info = std::get<1>(imgList[index]);
        bool &loaded = std::get<2>(imgList[index]);
        if (loaded) {
            if (img.isNull()) {
                this->remove(index);
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

QImage QHotLoadImageBay::random() {
    internalRandom();
    this->lastDirection.store(D_NEUTRAL);
    return current();
}

QImage QHotLoadImageBay::skipTo(QFileInfo fi) {
    for (int i = 0; i < imgList.length(); i++) {
        const QFileInfo &lfi = std::get<1>(imgList.at(i));
        if (lfi == fi) {
            index = i;
            this->internalSettleIndex();
            emit imageChanged(lfi.fileName());
            return current();
        }
    }
    return current();
}

void QHotLoadImageBay::Activate() {
    activationState = true;
}

void QHotLoadImageBay::Deactivate() {
    activationState = false;
    for (int i = 0; i < imgList.length(); i++) {
        if (i == index) continue;
        this->unload(i);
    }
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
            this->remove(i);
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

void QHotLoadImageBay::unload(int i) {
    bool &loaded = std::get<2>(imgList[i]);
    if (loaded) {
        QImage &img = std::get<0>(imgList[i]);
        bytesLoaded -= img.width() * img.height() * 4;
        img = QImage();
        loaded = false;
    }
}

void QHotLoadImageBay::remove(int i) {
    imgList.removeAt(i);
    this->internalSettleIndex();
}
