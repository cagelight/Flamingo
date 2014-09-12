#include <QtWidgets>
#include "qw_mainwindow.hpp"
#include "qw_imgview.hpp"

FlamingoMainWindow::FlamingoMainWindow() : QWidget(0) {
    //Initialization
    layoutMain = new QGridLayout(this);
    iview = new QImageView(this);
    QImage tImg = QImage("test.jpg");
    iview->setImage(tImg);
    //Layouting
    layoutMain->addWidget(iview);
}
