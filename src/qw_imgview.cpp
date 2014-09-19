#include <QtGui>
#include "qw_imgview.hpp"

QImageView::QImageView(QWidget *parent, QPixmap image) : QWidget(parent), view(image) {
    this->rasterTimer.setSingleShot(true);
    QObject::connect(&rasterTimer, SIGNAL(timeout()), this, SLOT(bilinearRaster()));
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
            drawSize = partRect.size().scaled(this->size(), Qt::IgnoreAspectRatio);
        } else if (partRect.width() < this->width() && partRect.height() < this->height() && (zoom >= 1.0f || zoom == zoomMax)) {
            drawSize = partRect.size();
        } else {
            drawSize = partRect.size().scaled(this->size(), Qt::KeepAspectRatio);
        }
        float widgetRatio, pixmapRatio;
        widgetRatio = this->width() / (float) this->height();
        pixmapRatio = drawSize.width() / drawSize.height();
        QRect drawRect;
        drawRect.setX((int)((this->width() - drawSize.width()) / 2.0f));
        drawRect.setY((int)((this->height() - drawSize.height()) / 2.0f));
        drawRect.setSize(drawSize);
        if (fastRaster) {
            paint.drawPixmap(drawRect, view, partRect);
        } else {
            QPixmap view2 = view.copy(partRect).scaled(drawRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            paint.drawPixmap(drawRect, view2);
        }
    }
}

void QImageView::resizeEvent(QResizeEvent *) {
    this->calculateMax();
    this->bilinearRasterDelayed();
}

void QImageView::wheelEvent(QWheelEvent *QWE) {
    QWE->accept();
    this->setZoom((1.0f - QWE->delta() / 360.0f / 1.5f) * zoom, QPointF(QWE->pos().x() / (float)this->width() , QWE->pos().y() / (float)this->height()));
}

void QImageView::mousePressEvent(QMouseEvent *QME) {
    if (QME->button() == Qt::LeftButton) {
        this->mouseMoving = true;
        this->prevMPos = QME->pos();
    }
    if (QME->button() == Qt::MiddleButton) {
        this->setZoom(1.0f, QPointF(QME->pos().x() / (float)this->width() , QME->pos().y() / (float)this->height()));
    }
}

void QImageView::mouseReleaseEvent(QMouseEvent *QME) {
    if (QME->button() == Qt::LeftButton) {
        this->mouseMoving = false;
    }
}

void QImageView::mouseMoveEvent(QMouseEvent *QME) {
    if (mouseMoving) {
        QPointF nPosAdj = ((QPointF)prevMPos - (QPointF)QME->pos()) * zoom;
        QPointF prevView = viewOffset;
        this->viewOffset.rx() += nPosAdj.x();
        this->viewOffset.ry() += nPosAdj.y();
        this->calculateView();
        if (viewOffset.toPoint() != prevView.toPoint()) {
            this->update();
            this->bilinearRasterDelayed();
        }
        this->prevMPos = QME->pos();
    }
}

void QImageView::setImage(const QImage &newView) {
    this->view = QPixmap::fromImage(newView);
    this->keepFit = true;
    this->repaint();
    //this->bilinearRaster(true);
}

void QImageView::setImage(const QPixmap &newView) {
    this->view = newView;
    this->keepFit = true;
    this->repaint();
    //this->bilinearRaster(true);
}

void QImageView::bilinearRaster(bool forceZoomed) {
    if (zoom != 1.0f || forceZoomed) {
        fastRaster = false;
        this->repaint();
        fastRaster = true;
    }
}

void QImageView::bilinearRasterDelayed(int msec) {
    if (rasterTimer.isActive()) {
        rasterTimer.stop();
        rasterTimer.start(msec);
    } else {
        rasterTimer.start(msec);
    }
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
        this->bilinearRasterDelayed();
    } else if (keepFit == false && zoom == nZoom && zoom != zoomMin) {
        keepFit = true;
        this->repaint();
        this->bilinearRasterDelayed();
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
