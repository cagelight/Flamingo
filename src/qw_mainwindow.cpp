#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoArgumentList infos) : FlamingoMainWindow() {
    fview = new QFlamingoView(infos, this);
    fview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutMain->addWidget(fview, 0, 0, 1, 2);
    QObject::connect(fview, SIGNAL(statusUpdate(QString)), this, SLOT(handleStatusUpdate(QString)));
    ssTimer->setTimerType(Qt::VeryCoarseTimer);
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    this->setMinimumSize(400, 300);
    this->setGeometry(QApplication::desktop()->availableGeometry());
    ssSpin->setMinimum(1);
    ssSpin->setMaximum(std::numeric_limits<int>::max() / 1000);
    QObject::connect(ssStart, SIGNAL(released()), this, SLOT(startSlideshow()));
    //Layouting
    layoutSS->setMargin(4);
    layoutSS->addWidget(ssLabel);
    layoutSS->addWidget(ssSpin);
    layoutSS->addWidget(ssLabelSec);
    layoutSS->addWidget(ssShuffle);
    layoutSS->addWidget(ssStart);
    widgetSS->setLayout(layoutSS);
    layoutMain->addWidget(widgetSS, 1, 0);
    layoutMain->addWidget(istatbar, 1, 1);
    layoutMain->setMargin(0);
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
        this->stopSlideshow();
        fview->Next();
        break;
    case Qt::Key_Left:
        this->stopSlideshow();
        fview->Prev();
        break;
    case Qt::Key_R:
        this->stopSlideshow();
        fview->Rand();
        break;
    case Qt::Key_H:
        if(widgetSS->isHidden()) {
            this->istatbar->show();
            this->widgetSS->show();
        } else {
            this->istatbar->hide();
            this->widgetSS->hide();
        }
        break;
    case Qt::Key_S:
        if (ssTimer->isActive())
            this->stopSlideshow();
        else
            this->startSlideshow();
        break;
    }
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

void FlamingoMainWindow::handleStatusUpdate(QString str) {
    this->istatbar->showMessage(str);
}

void FlamingoMainWindow::startSlideshow() {
    ssStart->disconnect();
    QObject::connect(ssStart, SIGNAL(released()), this, SLOT(stopSlideshow()));
    ssStart->setText("Stop");
    if (!ssTimer->isActive()) {
        QPalette pal = ssStart->palette();
        pal.setColor(QPalette::Button, QColor(255, 0, 0));
        ssStart->setPalette(pal);
    } else {
        ssTimer->stop();
        ssTimer->disconnect();
    }
    if (ssShuffle->isChecked()) {
        QObject::connect(ssTimer, SIGNAL(timeout()), fview, SLOT(Rand()));
    } else {
        QObject::connect(ssTimer, SIGNAL(timeout()), fview, SLOT(Next()));
    }
    ssTimer->start(ssSpin->value() * 1000);
}

void FlamingoMainWindow::stopSlideshow() {
    ssStart->disconnect();
    QObject::connect(ssStart, SIGNAL(released()), this, SLOT(startSlideshow()));
    ssStart->setText("Start");
    if (ssTimer->isActive()) {
        ssStart->setPalette(QApplication::palette());
        ssTimer->stop();
        ssTimer->disconnect();
    }
}
