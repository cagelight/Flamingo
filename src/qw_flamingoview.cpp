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
                QTrackedImage *nFile = new QTrackedImage(info, true);
                if (nFile->canRead()) {
                    this->imgCur = nFile;
                    return;
                } else delete nFile;
            }
        }
    }
    if (imgDirs.length() == 0 && imgFiles.length() == 0) {
        imgDirs.append(QDir::current());
    }
    if (imgCur == nullptr) imgCur = this->advanceImage(imgCur, RESET);
    imgNext = this->advanceImage(imgCur, NEXT);
    imgPrev = this->advanceImage(imgCur, NEXT, true);
    this->setImageCurrent();
}

void QFlamingoView::Next() {
    qDebug() << imgPrev->Info()->canonicalFilePath() << imgCur->Info()->canonicalFilePath() << imgNext->Info()->canonicalFilePath();
    imgPrev = imgCur;
    imgCur = imgNext;
    delete imgNext;
    imgNext = this->advanceImage(imgCur, NEXT);
    imgNext->beginRead();
    this->setImageCurrent();
}

void QFlamingoView::Prev() {
    imgNext = imgCur;
    imgCur = imgPrev;
    delete imgPrev;
    imgPrev = this->advanceImage(imgCur, NEXT);
    imgPrev->beginRead();
    this->setImageCurrent();
}


QTrackedImage* QFlamingoView::advanceImage(const QTrackedImage *begin, INFONAV method, bool reverse) {
    qDebug() << method;
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
        if (imgCur->isTracked()) return advanceImage(begin, NEXT_DIR_FILE, reverse); else return advanceImage(begin, NEXT_FILE, reverse);
        break;
    case NEXT_FILE:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while(fiter.canAdvance()) {
                QFileInfo &fcur = fiter.advance();
                if (fcur.canonicalFilePath() == imgCur->Info()->canonicalFilePath()) {
                    if (fiter.canAdvance()) {
                        QTrackedImage *nFile = new QTrackedImage(fiter.peekAdvance(), false);
                        if (!nFile->canRead()) {
                            delete nFile;
                            fiter.remove();
                            return advanceImage(begin, NEXT_FILE, reverse);
                        } else return nFile;
                    } else {
                        return advanceImage(begin, FORCE_DIR, reverse);
                    }
                }
            }
            imgFiles.removeAll(*(imgCur->Info()));
            return advanceImage(begin, RESET, reverse);
        } else {
            return advanceImage(begin, FORCE_DIR, reverse);
        }
        break;
    case NEXT_DIR_FILE:
        if (imgCur->Info()->dir().exists()) {
            QFileInfoList qfil = imgCur->Info()->dir().entryInfoList(QDir::Files);
            QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
            while(fiter.canAdvance()) {
                const QFileInfo &fcur = fiter.advance();
                if (fcur.canonicalFilePath() == imgCur->Info()->canonicalFilePath()) {matchFlag = true; continue;}
                if (matchFlag) {
                    QTrackedImage *nFile = new QTrackedImage(fcur, true);
                    if (nFile->canRead()) return nFile; else delete nFile;
                }
            }
            return advanceImage(begin, RESET_DIR, reverse);
        } else {
            imgDirs.removeAll(imgCur->Info()->dir());
            return advanceImage(begin, NEXT_DIR, reverse);
        }
        break;
    case RESET_DIR:
        if (imgCur->Info()->dir().exists()) {
            QFileInfoList qfil = imgCur->Info()->dir().entryInfoList(QDir::Files);
            QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
            while(fiter.canAdvance()) {
                QTrackedImage *nFile = new QTrackedImage(fiter.advance(), true);
                if (nFile->canRead()) return nFile; else delete nFile;
            }
            return advanceImage(begin, NEXT_DIR, reverse);
        }
        return advanceImage(begin, NEXT_DIR, reverse);
        break;
    case NEXT_DIR:
        if (imgDirs.length() > 0) {
            QReversibleMutableListIterator<QDir> diter(imgDirs, reverse);
            while(diter.canAdvance()) {
                QDir &dir = diter.advance();
                if (dir.exists()) {
                    if (dir == imgCur->Info()->dir()) {matchFlag = true; continue;}
                    if (matchFlag) {
                        QFileInfoList qfil = dir.entryInfoList(QDir::Files);
                        QReversibleListIterator<QFileInfo> fiter(qfil, reverse);
                        while(fiter.canAdvance()) {
                            QTrackedImage *nFile = new QTrackedImage(fiter.advance(), true);
                            if (!nFile->canRead()) {
                                delete nFile;
                                continue;
                            } else return nFile;
                        }
                    }
                } else {
                    diter.remove();
                }
            }
            return advanceImage(begin, FORCE_FILE, reverse);
        } else {
            return advanceImage(begin, FORCE_FILE, reverse);
        }
        break;
    case FORCE_FILE:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while (fiter.canAdvance()) {
                QTrackedImage *nFile = new QTrackedImage(fiter.advance(), false);
                if (!nFile->canRead()) {
                    delete nFile;
                    fiter.remove();
                    continue;
                } else return nFile;
            }
            return advanceImage(begin, RESET, reverse);
        } else {
            return advanceImage(begin, RESET, reverse);
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
                        QTrackedImage *nFile = new QTrackedImage(fiter.advance(), true);
                        if (!nFile->canRead()) {
                            delete nFile;
                            continue;
                        } else return nFile;
                    }
                } else {
                    diter.remove();
                }
            }
            return advanceImage(begin, RESET, reverse);
        } else {
            return advanceImage(begin, RESET, reverse);
        }
        break;
    case RESET:
        if (reverse) goto dirs; else goto files;
        files:
        if (imgFiles.length() > 0) {
            QReversibleMutableListIterator<QFileInfo> fiter(imgFiles, reverse);
            while(fiter.canAdvance()) {
                QTrackedImage *nFile = new QTrackedImage(fiter.advance(), false);
                if (!nFile->canRead()) {
                    delete nFile;
                    fiter.remove();
                    continue;
                } else return nFile;
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
                        QTrackedImage *nFile = new QTrackedImage(fiter.advance(), true);
                        if (!nFile->canRead()) {
                            delete nFile;
                            continue;
                        } else return nFile;
                    }
                } else {
                    diter.remove();
                }
            }
        }
        if (reverse) goto files; else goto fail;
        fail:
        //No files explicitly set, and no files in the dirs detected (or no dirs explicitly set): make blank.
        return nullptr;
        break;
    }
}

void QFlamingoView::setImageCurrent() {
    this->setImage(imgCur->getImage());
}
