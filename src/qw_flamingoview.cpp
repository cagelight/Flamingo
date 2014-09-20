#include "qw_flamingoview.hpp"

#include <QDebug>
#include <QDir>
#include <QImageReader>

QFlamingoView::QFlamingoView(QFileInfoList fi, QWidget *parent) : QImageView(parent) {
    QObject::connect(&qhlib, SIGNAL(activeStatusUpdate(QString)), this, SLOT(handleQHLIStatus(QString)));
    QObject::connect(&qhlib, SIGNAL(activeLoaded(QImage)), this, SLOT(flamSetImage(QImage)));
    if (fi.length() == 0) {
        QDir dir = QDir::current();
        for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
            qhlib.add(file);
        }
    } else if (fi.length() == 1) {
        const QFileInfo &file = fi.at(0);
        if (file.isFile()) {
            QDir dir = file.dir();
            for (QFileInfo file2 : dir.entryInfoList(QDir::Files)) {
                qhlib.add(file2);
            }
            qhlib.skipTo(file);
        } else {
            QDir dir = QDir(file.canonicalFilePath());
            for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                qhlib.add(file);
            }
        }
    } else {
        for (QFileInfo info : fi) {
            if (info.exists()) {
                if (info.isFile()) {
                    qhlib.add(info);
                } else {
                    QDir dir = QDir(info.canonicalFilePath());
                    for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                        qhlib.add(file);
                    }
                }
            }
        }

    }
    this->setImage(qhlib.current());
}

void QFlamingoView::Next() {;
    this->flamSetImage(qhlib.next());
}

void QFlamingoView::Prev() {
    this->flamSetImage(qhlib.previous());
}

void QFlamingoView::handleQHLIStatus(QString str) {
    emit statusUpdate(str);
}

void QFlamingoView::flamSetImage(QImage img) {
    if (img != this->getImage()) {
        this->setImage(img);
    }
}
