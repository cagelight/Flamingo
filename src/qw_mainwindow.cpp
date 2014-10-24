#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFlamingoLoadInformationData infos) {
    this->setMinimumSize(400, 300);
    this->setMenuBar(menu);
    this->setStatusBar(istatbar);
    //Menu Bar
    slideshowIntervalDialog->setLayout(new QGridLayout(slideshowIntervalDialog));
    slideshowIntervalSpinbox->setMinimum(1);
    slideshowIntervalSpinbox->setMaximum(std::numeric_limits<int>::max() / 1000);
    slideshowIntervalSpinbox->setValue(appSettings.value("slideshow_interval", QVariant(1)).value<int>());
    slideshowIntervalDialog->layout()->addWidget(slideshowIntervalSpinbox);
    QPushButton * slideshowIntervalCloseButton = new QPushButton("OK", slideshowIntervalDialog);
    slideshowIntervalDialog->layout()->addWidget(slideshowIntervalCloseButton);
    QObject::connect(slideshowIntervalCloseButton, SIGNAL(released()), slideshowIntervalDialog, SLOT(close()));
    viewLoadKeepDialog->setLayout(new QGridLayout(viewLoadKeepDialog));
    viewLoadKeepDialog->layout()->addWidget(viewLoadKeepFit);
    viewLoadKeepDialog->layout()->addWidget(viewLoadKeepFitForce);
    viewLoadKeepDialog->layout()->addWidget(viewLoadKeepExpanded);
    viewLoadKeepDialog->layout()->addWidget(viewLoadKeepEquals);

    QPushButton * viewLoadOk = new QPushButton("Close", viewLoadKeepDialog);
    viewLoadKeepDialog->layout()->addWidget(viewLoadOk);
    QObject::connect(viewLoadOk, SIGNAL(released()), viewLoadKeepDialog, SLOT(close()));

    QMenu * fileMenu = menu->addMenu("&File");
    QAction * closeAction = fileMenu->addAction("Close");
    closeAction->setShortcut(Qt::Key_Escape);
    QObject::connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    fileArgAction = fileMenu->addAction("Load Manager");
    fileArgAction->setShortcut(Qt::Key_M);

    QMenu * mwindowmenu = menu->addMenu("&Window");
    QAction * mwindowHide = mwindowmenu->addAction("Hide");
    mwindowHide->setShortcut(Qt::Key_H);
    QObject::connect(mwindowHide, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    QAction * mwindowFullscreen = mwindowmenu->addAction("Fullscreen");
    mwindowFullscreen->setCheckable(true);
    mwindowFullscreen->setShortcut(Qt::Key_F);
    QObject::connect(mwindowFullscreen, SIGNAL(toggled(bool)), this, SLOT(toggleFullscreen(bool)));
    mwindowBorder = mwindowmenu->addAction("Borderless");
    mwindowBorder->setCheckable(true);
    mwindowBorder->setShortcut(Qt::Key_B);
    QObject::connect(mwindowBorder, SIGNAL(toggled(bool)), this, SLOT(toggleBorder(bool)));

    QMenu * viewMenu = menu->addMenu("&View");
    viewKeepFit = viewMenu->addAction("Fit (Don't Zoom)");
    viewKeepFit->setShortcut(Qt::Key_1);
    viewKeepFitForce = viewMenu->addAction("Fit (Zoom to Fit)");
    viewKeepFitForce->setShortcut(Qt::Key_2);
    viewKeepExpanded = viewMenu->addAction("Expand to Fill");
    viewKeepExpanded->setShortcut(Qt::Key_3);
    viewKeepEquals = viewMenu->addAction("1:1 Center");
    viewKeepEquals->setShortcut(Qt::Key_4);
    QAction * viewLoadKeepAction = viewMenu->addAction("Set Initial Zoom");
    viewLoadKeepAction->setShortcut(Qt::Key_Z);
    QObject::connect(viewLoadKeepAction, SIGNAL(triggered()), this, SLOT(showViewLoadDialog()));

    QMenu * navigationMenu = menu->addMenu("&Navigation");
    QAction * navigationNext = navigationMenu->addAction("Next");
    navigationNext->setShortcut(keyNavigationNext);
    QObject::connect(navigationNext, SIGNAL(triggered()), this, SLOT(next()));
    QAction * navigationPrev = navigationMenu->addAction("Previous");
    navigationPrev->setShortcut(keyNavigationPrev);
    QObject::connect(navigationPrev, SIGNAL(triggered()), this, SLOT(previous()));
    QAction * navigationRand = navigationMenu->addAction("Random");
    navigationRand->setShortcut(keyNavigationRand);
    QObject::connect(navigationRand, SIGNAL(triggered()), this, SLOT(random()));

    QMenu * slideshowMenu = menu->addMenu("&Slideshow");
    slideshowStartAction = slideshowMenu->addAction("Start");
    slideshowStartAction->setShortcut(Qt::Key_S);
    QObject::connect(slideshowStartAction, SIGNAL(triggered()), this, SLOT(toggleSlideshow()));
    slideRandAction = slideshowMenu->addAction("Shuffle");
    slideRandAction->setShortcut(Qt::Key_R);
    QAction * slideIntervalAction = slideshowMenu->addAction("Interval");
    slideIntervalAction->setShortcut(Qt::Key_I);
    QObject::connect(slideIntervalAction, SIGNAL(triggered()), slideshowIntervalDialog, SLOT(show()));
    slideRandAction->setCheckable(true);
    slideRandAction->setChecked(appSettings.value("slideshow_shuffle", QVariant(false)).value<bool>());

    fview = new QFlamingoView(this);
    fview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setCentralWidget(fview);
    QObject::connect(fview, SIGNAL(statusUpdate(QString)), this, SLOT(handleStatusUpdate(QString)));
    QObject::connect(fview, SIGNAL(imageChanged(QString)), this, SLOT(handleImageChange(QString)));
    fview->processArgumentList(infos);
    argManager = new QFlamingoArgManager(infos, this);
    QObject::connect(argManager, SIGNAL(updateFinalized(QFileInfoArgumentList)), this, SLOT(reloadArguments(QFileInfoArgumentList)));
    QObject::connect(fileArgAction, SIGNAL(triggered()), argManager, SLOT(show()));
    QObject::connect(slideshowTimer, SIGNAL(timeout()), this, SLOT(slideshowNext()));
    slideshowTimer->setTimerType(Qt::VeryCoarseTimer);
    QObject::connect(viewKeepFit, SIGNAL(triggered()), fview, SLOT(setKeepFit()));
    QObject::connect(viewKeepFitForce, SIGNAL(triggered()), fview, SLOT(setKeepFitForce()));
    QObject::connect(viewKeepExpanded, SIGNAL(triggered()), fview, SLOT(setKeepExpanded()));
    QObject::connect(viewKeepEquals, SIGNAL(triggered()), fview, SLOT(setKeepEqual()));
    QObject::connect(viewLoadKeepFit, SIGNAL(released()), fview, SLOT(setLoadKeepFit()));
    QObject::connect(viewLoadKeepFitForce, SIGNAL(released()), fview, SLOT(setLoadKeepFitForce()));
    QObject::connect(viewLoadKeepExpanded, SIGNAL(released()), fview, SLOT(setLoadKeepExpanded()));
    QObject::connect(viewLoadKeepEquals, SIGNAL(released()), fview, SLOT(setLoadKeepEqual()));
    fview->setFocus();
    fview->setLoadKeep((QImageView::ZKEEP)appSettings.value("fview_init_load", QVariant(QImageView::KEEP_FIT)).value<unsigned int>());

    this->setAcceptDrops(true);

    QRect dRect = QApplication::desktop()->screenGeometry();
    dRect.setSize(dRect.size() * 0.85);
    dRect.moveCenter(QApplication::desktop()->screenGeometry().center());
    this->setGeometry(appSettings.value("mainwindow_geometry", QVariant(dRect)).value<QRect>());

    if (appSettings.value("mainwindow_maximized", QVariant(false)).value<bool>())
        this->showMaximized();
    else
        this->show();
}

FlamingoMainWindow::~FlamingoMainWindow() {
    appSettings.setValue("fview_init_load", QVariant((unsigned int)fview->getLoadKeepState()));
    appSettings.setValue("slideshow_shuffle", QVariant(slideRandAction->isChecked()));
    appSettings.setValue("slideshow_interval", QVariant(slideshowIntervalSpinbox->value()));
    appSettings.setValue("mainwindow_maximized", QVariant(this->isMaximized()));
    if (!this->isMaximized()) {
        appSettings.setValue("mainwindow_geometry", QVariant(this->geometry()));
    }
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

void FlamingoMainWindow::dragEnterEvent(QDragEnterEvent * QDEE) {
    QDEE->acceptProposedAction();
}

void FlamingoMainWindow::dragMoveEvent(QDragMoveEvent * QDME) {
    QDME->acceptProposedAction();
}

void FlamingoMainWindow::dragLeaveEvent(QDragLeaveEvent * QDLE) {
    QDLE->accept();
}

void FlamingoMainWindow::dropEvent(QDropEvent * QDE) {
    QDE->accept();
    if (QDE->mimeData()->hasUrls()) {
        QUrl flidURL;
        for (QUrl const & url : QDE->mimeData()->urls()) {
            if (QFlamingoLoadInformationData::fileIsFLID(url.toLocalFile()))
                flidURL = url; break;
        }
        if (flidURL.isEmpty()) {
            QFlamingoLoadInformationData qflid;
            for (QUrl const & url : QDE->mimeData()->urls()) {
                qflid.append(QFileInfoArgument(url.toLocalFile()));
            }
            argManager->showWithNewArgs(qflid);
        } else {
            QFlamingoLoadInformationData qflid = QFlamingoLoadInformationData::load(flidURL.toLocalFile());
            argManager->showWithNewArgs(qflid);
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

void FlamingoMainWindow::showViewLoadDialog() {
    this->viewLoadKeepDialog->show();
    switch (fview->getLoadKeepState()) {
    case QImageView::KEEP_FIT:
        viewLoadKeepFit->setChecked(true);
        break;
    case QImageView::KEEP_FIT_FORCE:
        viewLoadKeepFitForce->setChecked(true);
        break;
    case QImageView::KEEP_EXPANDED:
        viewLoadKeepExpanded->setChecked(true);
        break;
    case QImageView::KEEP_EQUAL:
        viewLoadKeepEquals->setChecked(true);
        break;
    default:
        viewLoadKeepFit->setChecked(false);
        viewLoadKeepFitForce->setChecked(false);
        viewLoadKeepExpanded->setChecked(false);
        viewLoadKeepEquals->setChecked(false);
        break;
    }
}
