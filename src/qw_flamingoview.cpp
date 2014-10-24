#include "qw_flamingoview.hpp"
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

QFlamingoView::QFlamingoView(QWidget *parent) : QImageView(parent) {
    QObject::connect(&qhlib, SIGNAL(imageChanged(QString)), this, SIGNAL(imageChanged(QString)));
    QObject::connect(&qhlib, SIGNAL(activeStatusUpdate(QString)), this, SLOT(handleQHLIStatus(QString)));
    QObject::connect(&qhlib, SIGNAL(activeLoaded(QImage)), this, SLOT(flamSetImage(QImage)));
}

void QFlamingoView::processArgumentList(QFileInfoArgumentList fi) {
    QtConcurrent::run(this, &QFlamingoView::internalProcessArgs, fi);
}

void QFlamingoView::internalProcessArgs(QFileInfoArgumentList fi) {
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
            QFileInfoList qfil;
            recurseThroughQDir(qfil, dir, file.isRecursive() ? std::numeric_limits<int>::max() : 1);
        }
    } else {
        for (QFileInfoArgument info : fi) {
            if (info.exists()) {
                if (info.isFile()) {
                    qhlib.add(info);
                } else {
                    QDir dir = QDir(info.canonicalFilePath());
                    QFileInfoList qfil;
                    recurseThroughQDir(qfil, dir, info.isRecursive() ? std::numeric_limits<int>::max() : 1);
                }
            }
        }
    }
}

void QFlamingoView::recurseThroughQDir(QFileInfoList &qfil, const QDir &D, int iter) {
    for (QFileInfo file : D.entryInfoList(QDir::Files)) {
        qhlib.add(file);
    }
    iter--;
    if (iter <= 0) return;
    emit statusUpdate(QString(D.canonicalPath()));
    for (QFileInfo infoDir : D.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (abort) return;
        QDir dir = QDir(infoDir.canonicalFilePath());
        recurseThroughQDir(qfil, dir, iter);
    }
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
        this->setImage(img, loadKeep);
    }
}
