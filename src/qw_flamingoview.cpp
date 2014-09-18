#include "qw_flamingoview.hpp"

#include <QDebug>
#include <QDir>
#include <QImageReader>

QFlamingoView::QFlamingoView(QFileInfoList fi, QWidget *parent) : QImageView(parent) {
    for (QFileInfo info : fi) {
        if (info.exists()) {
            if (info.isFile()) {
                imgList.append(std::shared_ptr<QHotLoadImage> (new QHotLoadImage(info, this)));
            } else {
                QDir dir = QDir(info.canonicalFilePath());
                for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                    imgList.append(std::shared_ptr<QHotLoadImage>(new QHotLoadImage(file, this)));
                }
            }
        }
    }
    imgListIter = std::unique_ptr<QLoopingMutableListIterator<SharedQHLI>>(new QLoopingMutableListIterator<SharedQHLI>(imgList));
    this->Next();
}

void QFlamingoView::Next() {
    SharedQHLI &qhli = imgListIter->next();
    if (qhli->imgAvailable()) {
        QImage img = qhli->getImage();
        if (img.isNull()) {
            imgListIter->remove();
            this->Next();
            return;
        }
        this->setImage(img);
    } else if (qhli->hasFailed()) {
        imgListIter->remove();
        this->Next();
        return;
    } else emit busy(qhli->getName() + QString(" is loading..."));
    this->processLoads();
    return;
}

void QFlamingoView::Prev() {
    SharedQHLI &qhli = imgListIter->previous();
    if (qhli->imgAvailable()) {
        QImage img = qhli->getImage();
        if (img.isNull()) {
            imgListIter->remove();
            this->Prev();
            return;
        }
        this->setImage(img);
    } else emit busy(qhli->getName() + QString(" is loading..."));
    this->processLoads();
    return;
}

void QFlamingoView::delayedSet(QImage img) {
    qDebug() << "Set Attempt";
    QImage simg = img;
    if (imgListIter->value()->imgAvailable() && imgListIter->value()->getImage() == simg) {
        this->setImage(simg);
        emit ok();
    }
}

void QFlamingoView::loadFailed(const QHotLoadImage *obj) {
    if (imgListIter->value().get() == obj) {
        imgListIter->remove();
        this->Next();
    }
}

void QFlamingoView::processLoads() {
    QListIterator<SharedQHLI> loadIter(imgList);
    while (loadIter.hasNext()) {
        const SharedQHLI &cur = loadIter.next();
        if (cur == imgListIter->peekNext(1)) {cur->hotLoad(); QObject::connect(cur.get(), SIGNAL(LoadComplete(QImage)), this, SLOT(delayedSet(QImage)), Qt::UniqueConnection); continue;}
        if (cur == imgListIter->peekNext(0)) {cur->hotLoad(); QObject::connect(cur.get(), SIGNAL(LoadComplete(QImage)), this, SLOT(delayedSet(QImage)), Qt::UniqueConnection); continue;}
        if (cur == imgListIter->value()) {cur->hotLoad(); QObject::connect(cur.get(), SIGNAL(LoadComplete(QImage)), this, SLOT(delayedSet(QImage)), Qt::UniqueConnection); continue;}
        if (cur == imgListIter->peekPrevious(0)) {cur->hotLoad(); QObject::connect(cur.get(), SIGNAL(LoadComplete(QImage)), this, SLOT(delayedSet(QImage)), Qt::UniqueConnection); continue;}
        if (cur == imgListIter->peekPrevious(1)) {cur->hotLoad(); QObject::connect(cur.get(), SIGNAL(LoadComplete(QImage)), this, SLOT(delayedSet(QImage)), Qt::UniqueConnection); continue;}
        QObject::disconnect(cur.get(), SIGNAL(LoadComplete(QImage)), 0, 0);
        cur->hotTryUnload();
    }
}
