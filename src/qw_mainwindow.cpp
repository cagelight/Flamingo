#include <QtWidgets>
#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoList infos) : FlamingoMainWindow() {
    fview = new QFlamingoView(infos, this);
    QObject::connect(fview, SIGNAL(busy(QString)), this, SLOT(setStatusBusy(QString)));
    QObject::connect(fview, SIGNAL(ok()), this, SLOT(setStatusOK()));
    layoutMain->addWidget(fview, 0, 0);
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    this->setMinimumSize(400, 300);
    this->resize(800, 600);
    //Initialization
    layoutMain = new QGridLayout(this);
    iprogbar = new QProgressBar(this);
    iprogbar->setMaximum(1);
    //Layouting
    layoutMain->addWidget(iprogbar, 1, 0);
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
        fview->Next();
        break;
    case Qt::Key_Left:
        fview->Prev();
        break;
    default:
        break;
    }
}

void FlamingoMainWindow::setStatusBusy(QString msg) {
    this->iprogbar->setValue(0);
    this->iprogbar->setFormat(msg);
}

void FlamingoMainWindow::setStatusOK() {
    this->iprogbar->setValue(1);
    this->iprogbar->resetFormat();
}
