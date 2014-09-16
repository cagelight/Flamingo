#ifndef QW_IMGVIEW_HPP
#define QW_IMGVIEW_HPP

#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QTimer>

class QImageView : public QWidget {
    Q_OBJECT
public:
    QImageView(QWidget *parent = 0, QPixmap image = QPixmap(0, 0));
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
public slots:
    void setImage(const QImage&);
    void setImage(const QPixmap&);
private slots:
    void bilinearRaster(bool forceZoomed = false);
    void bilinearRasterDelayed(int msec = 150);
private: //Variables
    QPixmap view;
    bool fastRaster = true;
    QTimer rasterTimer;
    float zoom = 1.0f;
    QRect partRect;
    float zoomMin = 0.025f;
    float zoomMax = 0.0f;
    bool keepFit = true;
    QPointF viewOffset = QPointF(0, 0);
    QPoint prevMPos;
    bool mouseMoving = false;
    bool paintCompletePartial = false;
    QPointF focalPoint;
private: //Methods
    void setZoom(qreal, QPointF focus = QPointF(0, 0));
    void calculateMax();
    void calculateView();

};

#endif // QW_IMGVIEW_HPP
