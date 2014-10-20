#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"
#include "qw_flamingoargmanager.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoArgumentList infos) : FlamingoMainWindow() {
    this->showMaximized();
    fview = new QFlamingoView(this);
    fview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutMain->addWidget(fview, 1, 0, 1, 2);
    QObject::connect(fview, SIGNAL(statusUpdate(QString)), this, SLOT(handleStatusUpdate(QString)));
    QObject::connect(fview, SIGNAL(imageChanged(QString)), this, SLOT(handleImageChange(QString)));
    fview->processArgumentList(infos);
    argManager = new QFlamingoArgManager(infos, this);
    QObject::connect(argManager, SIGNAL(updateFinalized(QFileInfoArgumentList)), this, SLOT(reloadArguments(QFileInfoArgumentList)));
    QObject::connect(slideshowTimer, SIGNAL(timeout()), this, SLOT(slideshowNext()));
    slideshowTimer->setTimerType(Qt::VeryCoarseTimer);
    fview->show();
    fview->setFocus();
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    this->setMinimumSize(400, 300);
    //Menu Bar
    slideshowIntervalDialog->setLayout(new QGridLayout(this));
    slideshowIntervalSpinbox->setMinimum(1);
    slideshowIntervalSpinbox->setMaximum(std::numeric_limits<int>::max() / 1000);
    slideshowIntervalDialog->layout()->addWidget(slideshowIntervalSpinbox);
    QPushButton * slideshowIntervalCloseButton = new QPushButton("OK", slideshowIntervalDialog);
    slideshowIntervalDialog->layout()->addWidget(slideshowIntervalCloseButton);
    QObject::connect(slideshowIntervalCloseButton, SIGNAL(released()), slideshowIntervalDialog, SLOT(close()));

    QMenu * fileMenu = menu->addMenu("File");
    QAction * closeAction = fileMenu->addAction("Close");
    closeAction->setShortcut(Qt::Key_Escape);
    QObject::connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));

    QMenu * mwindowmenu = menu->addMenu("Window");
    QAction * mwindowHide = mwindowmenu->addAction("Hide");
    mwindowHide->setShortcut(Qt::Key_H);
    QObject::connect(mwindowHide, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    QAction * mwindowFullscreen = mwindowmenu->addAction("Fullscreen");
    mwindowFullscreen->setCheckable(true);
    mwindowFullscreen->setShortcut(Qt::CTRL + Qt::Key_F);
    QObject::connect(mwindowFullscreen, SIGNAL(toggled(bool)), this, SLOT(toggleFullscreen(bool)));
    mwindowBorder = mwindowmenu->addAction("Borderless");
    mwindowBorder->setCheckable(true);
    mwindowBorder->setShortcut(Qt::CTRL + Qt::Key_B);
    QObject::connect(mwindowBorder, SIGNAL(toggled(bool)), this, SLOT(toggleBorder(bool)));

    QMenu * navigationMenu = menu->addMenu("Navigation");
    QAction * navigationNext = navigationMenu->addAction("Next");
    navigationNext->setShortcut(keyNavigationNext);
    QObject::connect(navigationNext, SIGNAL(triggered()), this, SLOT(next()));
    QAction * navigationPrev = navigationMenu->addAction("Previous");
    navigationPrev->setShortcut(keyNavigationPrev);
    QObject::connect(navigationPrev, SIGNAL(triggered()), this, SLOT(previous()));
    QAction * navigationRand = navigationMenu->addAction("Random");
    navigationRand->setShortcut(keyNavigationRand);
    QObject::connect(navigationRand, SIGNAL(triggered()), this, SLOT(random()));

    QMenu * slideshowMenu = menu->addMenu("Slideshow");
    slideshowStartAction = slideshowMenu->addAction("Start");
    slideshowStartAction->setShortcut(Qt::SHIFT + Qt::Key_S);
    QObject::connect(slideshowStartAction, SIGNAL(triggered()), this, SLOT(toggleSlideshow()));
    slideRandAction = slideshowMenu->addAction("Shuffle");
    slideRandAction->setShortcut(Qt::SHIFT + Qt::Key_R);
    QAction * slideIntervalAction = slideshowMenu->addAction("Interval");
    slideIntervalAction->setShortcut(Qt::SHIFT + Qt::Key_I);
    QObject::connect(slideIntervalAction, SIGNAL(triggered()), slideshowIntervalDialog, SLOT(show()));
    slideRandAction->setCheckable(true);
    //Main
    layoutMain->addWidget(menu, 0, 0, 1, 2);
    layoutMain->addWidget(istatbar, 3, 0, 1, 2);
    layoutMain->setMargin(0);
}

void FlamingoMainWindow::closeEvent(QCloseEvent *QCE) {
    QWidget::closeEvent(QCE);
    emit closed();
}

void FlamingoMainWindow::wheelEvent(QWheelEvent * QWE) {
    if (QWE->orientation() == Qt::Horizontal) {
        QWE->accept();
        if (QWE->delta() < 0) {
            fview->Next();
        } else if (QWE->delta() > 0) {
            fview->Prev();
        }
    }
}

void FlamingoMainWindow::keyPressEvent(QKeyEvent *QKE) {
    if (QKE->key() == Qt::Key_A) {
        argManager->show();
    }
    if (istatbar->isHidden()) {
        switch (QKE->key()) {
        case Qt::Key_Escape:
        case Qt::Key_H:
            this->toggleHidden();
            break;
        case keyNavigationNext:
            this->next();
            break;
        case keyNavigationPrev:
            this->previous();
            break;
        case keyNavigationRand:
            this->random();
            break;
        }
    }
}

void FlamingoMainWindow::handleStatusUpdate(QString str) {
    this->istatbar->showMessage(str);
}

void FlamingoMainWindow::handleImageChange(QString str) {
    this->setWindowTitle(QString("Flamingo: ") + str);
}

void FlamingoMainWindow::startSlideshow() {
    slideshowStartAction->setText("Stop");
    if (slideshowTimer->isActive()) {
        slideshowTimer->stop();
    }
    slideshowTimer->start(slideshowIntervalSpinbox->value() * 1000);
}

void FlamingoMainWindow::stopSlideshow() {
    slideshowStartAction->setText("Start");
    slideshowTimer->stop();
}

void FlamingoMainWindow::toggleSlideshow() {
    if (slideshowTimer->isActive())
        this->stopSlideshow();
    else
        this->startSlideshow();
}

void FlamingoMainWindow::next() {
    this->stopSlideshow();
    fview->Next();
}

void FlamingoMainWindow::previous() {
    this->stopSlideshow();
    fview->Prev();
}

void FlamingoMainWindow::random() {
    this->stopSlideshow();
    fview->Rand();
}

void FlamingoMainWindow::toggleHidden() {
    if(istatbar->isHidden()) {
        this->istatbar->show();
        this->menu->setVisible(true);
    } else {
        this->istatbar->hide();
        this->menu->setHidden(true);
    }
}

void FlamingoMainWindow::toggleFullscreen(bool flag) {
    if (flag) {
        mwindowBorder->setEnabled(false);
        this->setWindowState(this->windowState() | Qt::WindowFullScreen);
    } else {
        mwindowBorder->setEnabled(true);
        this->setWindowState(this->windowState() & ~Qt::WindowFullScreen);
    }
    this->show();
}

void FlamingoMainWindow::toggleBorder(bool flag) {
    //if ((this->windowState() & Qt::WindowFullScreen) != Qt::WindowFullScreen) {
    if (flag)
        this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    else
        this->setWindowFlags(this->windowFlags() & ~Qt::FramelessWindowHint);
    this->show();
   // }
}

void FlamingoMainWindow::slideshowNext() {
    if (slideRandAction->isChecked()) {
        fview->Rand();
    } else {
        fview->Next();
    }
}

void FlamingoMainWindow::reloadArguments(QFileInfoArgumentList qfial) {
    fview->clear();
    fview->processArgumentList(qfial);
}
