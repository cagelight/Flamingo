#include "qw_flamingoview.hpp"
#include "qreversiblelistiterator.hpp"

#include <QDebug>
#include <QDir>
#include <QImageReader>

QFlamingoView::QFlamingoView(QFileInfoList fi, QWidget *parent) : QImageView(parent) {
    if (fi.length() > 1) {
        for (const QFileInfo &info : fi) {
            if (info.exists()) {
                if (info.isDir()) {
                    imgDirs.append(QDir(info.canonicalFilePath()));
                } else {
                    if (QImageReader(info.canonicalFilePath()).canRead()) {
                        imgFiles.append(info);
                    }
                }
            }
        }
    } else if (fi.length() == 1) {
        const QFileInfo &info = fi[0];
        if (info.exists()) {
            if (info.isDir()) {
                imgDirs.append(QDir(info.canonicalFilePath()));
            } else {
                imgDirs.append(info.dir());
                if (QImageReader(info.canonicalFilePath()).canRead() && setCurrent(QTrackedFile(info, true))) {
                    this->setImageCurrent();
                    return;
                }
            }
        }
    }
    if (imgDirs.length() == 0 && imgFiles.length() == 0) {
        imgDirs.append(QDir::current());
    }
    this->advanceImage(RESET);
    this->setImageCurrent();
}

void QFlamingoView::Next() {
    this->advanceImage(NEXT);
    this->setImageCurrent();
}

void QFlamingoView::Prev() {
    this->advanceImage(NEXT, true);
    this->setImageCurrent();
}


void QFlamingoView::advanceImage(INFONAV method, bool reverse) {
    //ENUM HIERARCHY
    //NEXT
    //-NEXT_FILE
    //--RESET
    //--FORCE_DIR
    //---RESET
    //-NEXT_FILE_DIR
    //--RESET_DIR
    //---NEXT_DIR
    //----FORCE_FILE
    //-----RESET
    //--NEXT_DIR
    //---FORCE_FILE
    //----RESET
    //RANDOM
    bool matchFlag = false;
    switch (method) {
    case NEXT:
        if (imgCur.isTracked()) advanceImage(NEXT_DIR_FILE, reverse); else advanceImage(NEXT_FILE, reverse);
        break;
    case NEXT_FILE:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while(fiter.canAdvance()) {
                QFileInfo &fcur = fiter.advance();
                if (fcur == imgCur) {
                    if (fiter.canAdvance()) {
                        QTrackedFile nFile = QTrackedFile(fiter.peekAdvance(), false);
                        if (!setCurrent(nFile)) {
                            fiter.remove();
                            advanceImage(NEXT_FILE, reverse);
                            return;
                        } else return;
                    } else {
                        advanceImage(FORCE_DIR, reverse);
                        return;
                    }
                }
            }
            imgFiles.removeAll(imgCur);
            advanceImage(RESET, reverse);
            return;
        } else {
            advanceImage(FORCE_DIR, reverse);
            return;
        }
        break;
    case NEXT_DIR_FILE:
        if (imgCur.dir().exists()) {
            QFileInfoList qfil = imgCur.dir().entryInfoList(QDir::Files);
            QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
            while(fiter.canAdvance()) {
                const QFileInfo &fcur = fiter.advance();
                if (fcur == imgCur) {matchFlag = true; continue;}
                if (matchFlag) {
                    if (setCurrent(QTrackedFile(fcur, true))) return;
                }
            }
            advanceImage(RESET_DIR, reverse);
            return;
        } else {
            imgDirs.removeAll(imgCur.dir());
            advanceImage(NEXT_DIR, reverse);
            return;
        }
        break;
    case RESET_DIR:
        if (imgCur.dir().exists()) {
            QFileInfoList qfil = imgCur.dir().entryInfoList(QDir::Files);
            QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
            while(fiter.canAdvance()) {
                if (setCurrent(QTrackedFile(fiter.advance(), true))) return;
            }
            advanceImage(NEXT_DIR, reverse);
            return;
        }
        advanceImage(NEXT_DIR, reverse);
        return;
        break;
    case NEXT_DIR:
        if (imgDirs.length() > 0) {
            QReversibleMutableListIterator<QDir> diter(imgDirs, reverse);
            while(diter.canAdvance()) {
                QDir &dir = diter.advance();
                if (dir.exists()) {
                    if (dir == imgCur.dir()) {matchFlag = true; continue;}
                    if (matchFlag) {
                        QFileInfoList qfil = dir.entryInfoList(QDir::Files);
                        QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
                        while(fiter.canAdvance()) {
                            if (!setCurrent(QTrackedFile(fiter.advance(), true))) {
                                continue;
                            } else return;
                        }
                    }
                } else {
                    diter.remove();
                }
            }
            advanceImage(FORCE_FILE, reverse);
            return;
        } else {
            advanceImage(FORCE_FILE, reverse);
            return;
        }
        break;
    case FORCE_FILE:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while (fiter.canAdvance()) {
                if (!setCurrent(QTrackedFile(fiter.advance(), false))) {
                    fiter.remove();
                    continue;
                } else return;
            }
            advanceImage(RESET, reverse);
            return;
        } else {
            advanceImage(RESET, reverse);
            return;
        }
        break;
    case FORCE_DIR:
        if (imgDirs.length() > 0) {
            QReversibleMutableListIterator<QDir> diter(imgDirs, reverse);
            while(diter.canAdvance()) {
                QDir &dir = diter.advance();
                if (dir.exists()) {
                    QFileInfoList qfil = dir.entryInfoList(QDir::Files);
                    QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
                    while(fiter.canAdvance()) {
                        if (!setCurrent(QTrackedFile(fiter.advance(), true))) {
                            continue;
                        } else return;
                    }
                } else {
                    diter.remove();
                }
            }
            advanceImage(RESET, reverse);
            return;
        } else {
            advanceImage(RESET, reverse);
            return;
        }
        break;
    case RESET:
        if (reverse) goto dirs; else goto files;
        files:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while(fiter.canAdvance()) {
                if (!setCurrent(QTrackedFile(fiter.advance(), false))) {
                    fiter.remove();
                    continue;
                } else return;
            }
        }
        if (reverse) goto fail; else goto dirs;
        dirs:
        if (imgDirs.length() > 0) {
            QReversibleMutableListIterator<QDir> diter(imgDirs, reverse);
            while(diter.canAdvance()) {
                QDir &dir = diter.advance();
                if (dir.exists()) {
                    QFileInfoList qfil = dir.entryInfoList(QDir::Files);
                    QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
                    while(fiter.canAdvance()) {
                        if (!setCurrent(QTrackedFile(fiter.advance(), true))) {
                            continue;
                        } else return;
                    }
                } else {
                    diter.remove();
                }
            }
        }
        if (reverse) goto files; else goto fail;
        fail:
        //No files explicitly set, and no files in the dirs detected (or no dirs explicitly set): make blank.
        imgCur = QTrackedFile();
        this->setImage(QImage(0, 0));
        break;
    }
}

bool QFlamingoView::setCurrent(QTrackedFile f) {
    if (f.exists() && QImageReader(f.canonicalFilePath()).canRead()) {
        imgCur = f;
        return true;
    }
    return false;
}

void QFlamingoView::setImageCurrent() {
    this->setImage(QImage(imgCur.canonicalFilePath()));
}
