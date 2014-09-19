#include "qw_flamingoview.hpp"

#include <QDebug>
#include <QDir>
#include <QImageReader>

QFlamingoView::QFlamingoView(QFileInfoList fi, QWidget *parent) : QImageView(parent) {
    QObject::connect(&qhlib, SIGNAL(activeLoaded(QImage)), this, SLOT(setImage(QImage)));
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
    this->setImage(qhlib.current());
}

void QFlamingoView::Next() {
    this->setImage(qhlib.next());
}

void QFlamingoView::Prev() {
    this->setImage(qhlib.previous());
}
