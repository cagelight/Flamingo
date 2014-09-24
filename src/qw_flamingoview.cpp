#include "qw_flamingoview.hpp"
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QtWidgets>

static void recurseThroughQDir(QFileInfoList &qfil, const QDir &D, int iter = std::numeric_limits<int>::max()) {
    for (QFileInfo file : D.entryInfoList(QDir::Files)) {
        qfil.append(file);
    }
    iter--;
    if (iter <= 0) return;
    for (QFileInfo infoDir : D.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir dir = QDir(infoDir.canonicalFilePath());
        recurseThroughQDir(qfil, dir, iter);
    }
}

QFlamingoView::QFlamingoView(QFileInfoArgumentList fi, QWidget *parent) : QImageView(parent) {
    QObject::connect(&qhlib, SIGNAL(activeStatusUpdate(QString)), this, SLOT(handleQHLIStatus(QString)));
    QObject::connect(&qhlib, SIGNAL(activeLoaded(QImage)), this, SLOT(flamSetImage(QImage)));
    if (fi.length() == 0) {
        QDir dir = QDir::current();
        for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
            qhlib.add(file);
        }
    } else if (fi.length() == 1) {
        const QFileInfoArgument &file = fi.at(0);

        if (file.isFile()) {
            QDir dir = file.dir();
            for (QFileInfo file2 : dir.entryInfoList(QDir::Files)) {
                qhlib.add(file2);
            }
            qhlib.skipTo(file);
        } else {
            QDir dir = QDir(file.canonicalFilePath());
            if (file.isRecursive()) {
                QFileInfoList qfil;
                recurseThroughQDir(qfil, dir);
                for (QFileInfo file2 : qfil)
                    qhlib.add(file2);
            }
            for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                qhlib.add(file);
            }
        }
    } else {
        for (QFileInfoArgument info : fi) {
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

void QFlamingoView::hideEvent(QHideEvent *) {
    qhlib.Deactivate();
}

void QFlamingoView::showEvent(QShowEvent *) {
    qhlib.Activate();
}

void QFlamingoView::Next() {
    QImage img = qhlib.next();
    if (!img.isNull()) this->flamSetImage(img);
}

void QFlamingoView::Prev() {
    QImage img = qhlib.previous();
    if (!img.isNull()) this->flamSetImage(img);
}

void QFlamingoView::Rand() {
    QImage img = qhlib.random();
    if (!img.isNull()) this->flamSetImage(img);
}

void QFlamingoView::handleQHLIStatus(QString str) {
    emit statusUpdate(str);
}

void QFlamingoView::flamSetImage(QImage img) {
    if (img != this->getImage()) {
        this->setImage(img);
    }
}
