#include <QtWidgets>
#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoList infos) : FlamingoMainWindow() {
    fview = new QFlamingoView(infos, this);
    fview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutMain->addWidget(fview, 0, 0);

    QObject::connect(fview, SIGNAL(statusUpdate(QString)), this, SLOT(handleStatusUpdate(QString)));
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    this->setMinimumSize(400, 300);
    this->resize(800, 600);
    //Initialization
    layoutMain = new QGridLayout(this);
    istatbar = new QStatusBar(this);
    //Layouting
    layoutMain->addWidget(istatbar, 1, 0);
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

void FlamingoMainWindow::wheelEvent(QWheelEvent * QWE) {
    if (QWE->orientation() == Qt::Horizontal) {
        QWE->accept();
        if (QWE->delta() > 0) {
            fview->Next();
        } else if (QWE->delta() < 0) {
            fview->Prev();
        }
    }
}

void FlamingoMainWindow::handleStatusUpdate(QString str) {
    this->istatbar->showMessage(str);
}
