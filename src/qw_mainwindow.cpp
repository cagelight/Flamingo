#include <QtWidgets>
#include "qw_mainwindow.hpp"
#include "qw_imgview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoList infos) : FlamingoMainWindow() {
    for (QFileInfo info : infos) {
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
    if (imgDirs.length() == 0 && imgFiles.length() == 0) {
        imgDirs.append(QDir::current());
    }
    this->advanceImage(RESET_BEGIN);
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    //Initialization
    layoutMain = new QGridLayout(this);
    iview = new QImageView(this);
    //Layouting
    layoutMain->addWidget(iview);
}

void FlamingoMainWindow::closeEvent(QCloseEvent *QCE) {
    QWidget::closeEvent(QCE);
    emit closed();
}

void FlamingoMainWindow::keyPressEvent(QKeyEvent *QKE) {
    switch(QKE->key()) {
    case Qt::Key_Escape:
        this->close();
        break;
    case Qt::Key_Right:
        this->advanceImage(NEXT);
    default:
        break;
    }
}

void FlamingoMainWindow::advanceImage() {
    this->advanceImage(NEXT);
}

void FlamingoMainWindow::advanceImage(INFONAV method) {
    //ENUM HIERARCHY
    //NEXT
    //-NEXT_FILE
    //--RESET_BEGIN
    //--FORCE_FIRST_DIR
    //---RESET_BEGIN
    //-NEXT_FILE_DIR
    //--NEXT_DIR
    //---FORCE_FIRST_FILE
    //----RESET_BEGIN
    //PREV
    //-PREV_FILE
    //--RESET_END
    //--FORCE_LAST_DIR
    //---RESET_END
    //-PREV_FILE_DIR
    //--PREV_DIR
    //---FORCE_LAST_FILE
    //----RESET_END
    //RANDOM
    bool matchFlag = false;
    switch (method) {
    case NEXT:
        if (imgCur.isTracked()) advanceImage(NEXT_DIR_FILE); else advanceImage(NEXT_FILE);
        break;
    case NEXT_FILE:
        if (imgFiles.length() > 0) {
            for (int i = 0; i < imgFiles.length(); i++) {
                if (imgFiles[i] == imgCur) {
                    if (i != imgFiles.length() - 1) {
                        QTrackedFile nFile = QTrackedFile(imgFiles[i+1], false);
                        if (!setCurrent(nFile)) {
                            imgFiles.removeAll(nFile);
                            advanceImage(NEXT_FILE);
                            return;
                        } else return;
                    } else {
                        advanceImage(FORCE_FIRST_DIR);
                        return;
                    }
                }
            }
            imgFiles.removeAll(imgCur);
            advanceImage(RESET_BEGIN);
            return;
        } else {
            advanceImage(FORCE_FIRST_DIR);
            return;
        }
        break;
    case NEXT_DIR_FILE:
        if (imgCur.dir().exists()) {
            for (QFileInfo file : imgCur.dir().entryInfoList(QDir::Files)) {
                if (file == imgCur) {matchFlag = true; continue;}
                if (matchFlag) {
                    if (setCurrent(QTrackedFile(file, true))) return;
                }
            }
            advanceImage(NEXT_DIR);
            return;
        } else {
            imgDirs.removeAll(imgCur.dir());
            advanceImage(NEXT_DIR);
            return;
        }
        break;
    case NEXT_DIR:
        if (imgDirs.length() > 0) {
            for (QDir dir : imgDirs) {
                if (dir.exists()) {
                    if (dir == imgCur.dir()) {matchFlag = true; continue;}
                    if (matchFlag) {
                        for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                            if (!setCurrent(QTrackedFile(file, true))) {
                                continue;
                            } else return;
                        }
                    }
                } else {
                    imgDirs.removeAll(dir);
                }
            }
            advanceImage(FORCE_FIRST_FILE);
            return;
        } else {
            advanceImage(FORCE_FIRST_FILE);
            return;
        }
        break;
    case FORCE_FIRST_FILE:
        if (imgFiles.length() > 0) {
            for (QFileInfo file : imgFiles) {
                if (!setCurrent(QTrackedFile(file, false))) {
                    imgFiles.removeAll(file);
                    continue;
                } else return;
            }
            advanceImage(RESET_BEGIN);
            return;
        } else {
            advanceImage(RESET_BEGIN);
            return;
        }
        break;
    case FORCE_FIRST_DIR:
        if (imgDirs.length() > 0) {
            for (QDir dir : imgDirs) {
                if (dir.exists()) {
                    for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                        if (!setCurrent(QTrackedFile(file, true))) {
                            continue;
                        } else return;
                    }
                } else {
                    imgDirs.removeAll(dir);
                }
            }
            advanceImage(RESET_BEGIN);
            return;
        } else {
            advanceImage(RESET_BEGIN);
            return;
        }
        break;
    case RESET_BEGIN:
        if (imgFiles.length() > 0) {
            for (QFileInfo file : imgFiles) {
                if (!setCurrent(QTrackedFile(file, false))) {
                    imgFiles.removeAll(file);
                    continue;
                } else return;
            }
        }
        if (imgDirs.length() > 0) {
            for (QDir dir : imgDirs) {
                if (dir.exists()) {
                    for (QFileInfo file : dir.entryInfoList(QDir::Files)) {
                        if (!setCurrent(QTrackedFile(file, true))) {
                            continue;
                        } else return;
                    }
                } else {
                    imgDirs.removeAll(dir);
                }
            }
        }
        //No files explicitly set, and no files in the dirs detected (or no dirs explicitly set): make blank.
        imgCur = QTrackedFile();
        iview->setImage(QImage(400, 300, QImage::Format_Mono));
        break;
    }
}

bool FlamingoMainWindow::setCurrent(QTrackedFile f) {
    if (f.exists() && QImageReader(f.canonicalFilePath()).canRead()) {
        imgCur = f;
        this->iview->setImage(QImage(f.canonicalFilePath()));
        return true;
    }
    return false;
}
