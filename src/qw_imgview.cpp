#include <QtGui>
#include "qw_imgview.hpp"

Q_DECLARE_METATYPE(DrawSet)

QImageView::QImageView(QWidget *parent, QImage image) : QWidget(parent), view(image), bilinearWorker(this) {
    qRegisterMetaType<DrawSet>("DrawSet");
    QObject::connect(&bilinearWorker, SIGNAL(done(DrawSet)), this, SLOT(handleBilinear(DrawSet)));
    mouseHider->setSingleShot(true);
    QObject::connect(mouseHider, SIGNAL(timeout()), this, SLOT(hideMouse()));
    this->setMouseTracking(true);
}

QSize QImageView::sizeHint() const {
    //return view.size().scaled(QSize(400, 300), Qt::KeepAspectRatioByExpanding);
    return view.size();
}

void QImageView::paintEvent(QPaintEvent *QPE) {
    QPainter paint(this);
    paint.fillRect(QPE->rect(), QBrush(QColor(0, 0, 0)));
    if (this->width() > 0 && this->height() > 0 && view.width() > 0 && view.height() > 0) {
        this->calculateView();
        QSize drawSize;
        if (paintCompletePartial) {
            drawSize = this->size();
        } else if (partRect.width() < this->width() && partRect.height() < this->height() && (zoom >= 1.0f || zoom == zoomMax)) {
            drawSize = partRect.size();
        } else {
            drawSize = partRect.size().scaled(this->size(), Qt::KeepAspectRatio);
        }
        QRect drawRect;
        drawRect.setX((int)((this->width() - drawSize.width()) / 2.0f));
        drawRect.setY((int)((this->height() - drawSize.height()) / 2.0f));
        drawRect.setSize(drawSize);
        {
            QRect &partRectBil = std::get<0>(bilinearObject);
            QRect &drawRectBil = std::get<1>(bilinearObject);
            QImage &imgOrig = std::get<2>(bilinearObject);
            QImage &imgBilPart = std::get<3>(bilinearObject);
            if (drawRect == drawRectBil && partRect == partRectBil && view == imgOrig) {
                paint.drawImage(drawRect, imgBilPart);
            } else {
                paint.drawImage(drawRect, view, partRect);
                QImage nil = QImage();
                bilinearWorker.render(std::tie<QRect, QRect, QImage, QImage>(partRect, drawRect, view, nil));
            }
        }
    }
    QWidget::paintEvent(QPE);
}

void QImageView::resizeEvent(QResizeEvent *QRE) {
    this->calculateMax();
    QWidget::resizeEvent(QRE);
}

void QImageView::wheelEvent(QWheelEvent *QWE) {
    if (QWE->orientation() == Qt::Vertical) {
        QWE->accept();
        this->setZoom((1.0f - QWE->delta() / 360.0f / 1.5f) * zoom, QPointF(QWE->pos().x() / (float)this->width() , QWE->pos().y() / (float)this->height()));
    }
    QWidget::wheelEvent(QWE);
}

void QImageView::mousePressEvent(QMouseEvent *QME) {
    this->showMouse();
    if (QME->button() == Qt::LeftButton) {
        QME->accept();
        this->setFocus();
        this->mouseMoving = true;
        this->prevMPos = QME->pos();
    }
    if (QME->button() == Qt::MiddleButton) {
        this->setZoom(1.0f, QPointF(QME->pos().x() / (float)this->width() , QME->pos().y() / (float)this->height()));
    }
    QWidget::mousePressEvent(QME);
}

void QImageView::mouseReleaseEvent(QMouseEvent *QME) {
    if (QME->button() == Qt::LeftButton) {
        this->mouseMoving = false;
    }
    QWidget::mouseReleaseEvent(QME);
}

void QImageView::mouseMoveEvent(QMouseEvent *QME) {
    this->showMouse();
    if (mouseMoving) {
        QPointF nPosAdj = ((QPointF)prevMPos - (QPointF)QME->pos()) * zoom;
        QPointF prevView = viewOffset;
        this->viewOffset.rx() += nPosAdj.x();
        this->viewOffset.ry() += nPosAdj.y();
        this->calculateView();
        if (viewOffset.toPoint() != prevView.toPoint()) {
            this->update();
        }
        this->prevMPos = QME->pos();
    }
    QWidget::mouseMoveEvent(QME);
}

void QImageView::setImage(const QImage &newView) {
    this->view = newView;
    this->keepFit = true;
    this->repaint();
}

void QImageView::setImage(QString const & path, bool threadedLoad) {
    if (threadedLoad) {
        if (delayedLoader != nullptr) {
            if (delayedLoader->joinable()) delayedLoader->join();
            delete delayedLoader;
            delayedLoader = nullptr;
        }
        delayedLoader = new std::thread(&QImageView::delayedLoad, this, path);
    }
    else
        this->setImage(QImage(path));
}

void QImageView::handleBilinear(DrawSet d) {
    this->bilinearObject = d;
    QRect &partRectBil = std::get<0>(bilinearObject);
    const QImage &imgOrig = std::get<2>(bilinearObject);
    const QImage &imgBil = std::get<3>(bilinearObject);
    if (partRect == partRectBil && view == imgOrig && !imgBil.isNull()) this->repaint();
}

void QImageView::setZoom(qreal nZoom, QPointF focus) {
    if (nZoom < zoomMin) nZoom = zoomMin;
    if (nZoom > zoomMax) nZoom = zoomMax;
    if (zoom != nZoom) {
        QSize sizeZ = this->size() * zoom;
        QSize sizeNZ = this->size() * nZoom;
        QSize sizeZV = this->view.size();
        if (sizeZV.width() > sizeZ.width()) {
            sizeZV.setWidth(sizeZ.width());
        }
        if (sizeZV.height() > sizeZ.height()) {
            sizeZV.setHeight(sizeZ.height());
        }
        float xmod = (sizeZV.width() - sizeNZ.width()) * focus.x();
        float ymod = (sizeZV.height() - sizeNZ.height()) * focus.y();
        this->viewOffset.rx() += xmod;
        this->viewOffset.ry() += ymod;
        zoom = nZoom;
        keepFit = false;
        this->repaint();
    } else if (keepFit == false && zoom == nZoom && zoom != zoomMin) {
        keepFit = true;
        this->repaint();
    }
}

void QImageView::calculateMax() {
    float xmax = view.width() / (float) this->width();
    float ymax = view.height() / (float) this->height();
    zoomMax = xmax > ymax ? xmax : ymax;
}

void QImageView::calculateView() {
    this->calculateMax();
    if (keepFit) {
        zoom = zoomMax;
        this->viewOffset = QPointF(0, 0);
    }
    QSize partSize = this->size() * zoom;
    this->paintCompletePartial = true;
    if (partSize.width() > view.width()) {
        partSize.setWidth(view.width());
        this->paintCompletePartial = false;
    }
    if (partSize.height() > view.height()) {
        partSize.setHeight(view.height());
        this->paintCompletePartial = false;
    }
    if (partSize == view.size()) keepFit = true;
    if (viewOffset.x() > this->view.width() - partSize.width()) {
        viewOffset.setX(this->view.width() - partSize.width());
    }
    if (viewOffset.y() > this->view.height() - partSize.height()) {
        viewOffset.setY(this->view.height() - partSize.height());
    }
    if (viewOffset.x() < 0) {
        viewOffset.setX(0);
    }
    if (viewOffset.y() < 0) {
        viewOffset.setY(0);
    }
    partRect = QRect(viewOffset.toPoint(), partSize);
}
