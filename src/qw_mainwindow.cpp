#include <QtWidgets>
#include "qw_mainwindow.hpp"
#include "qw_flamingoview.hpp"

FlamingoMainWindow::FlamingoMainWindow(QFileInfoList infos) : FlamingoMainWindow() {
    fview = new QFlamingoView(infos, this);
    layoutMain->addWidget(fview);
}

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    this->setMinimumSize(400, 300);
    //Initialization
    layoutMain = new QGridLayout(this);
    //Layouting
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
