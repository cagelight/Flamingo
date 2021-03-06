#ifndef QW_IMGVIEW_HPP
#define QW_IMGVIEW_HPP

#include <thread>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QTimer>

typedef std::tuple<QRect, QRect, QImage, QImage> DrawSet;

class QBilinearWorker : public QObject {
    Q_OBJECT
public:
    QBilinearWorker(QObject *parent = 0) : QObject(parent) {
        this->startTimer(10);
    }
    ~QBilinearWorker() {
        if (worker != nullptr) {
            if (worker->joinable()) worker->join();
            delete worker;
            worker = nullptr;
        }
    }
    void timerEvent(QTimerEvent *) {
        if (doRender) {
            if (worker == nullptr) {
                doRender = false;
                worker = new std::thread(std::bind(&QBilinearWorker::threadRun, this, curSet));
            } else {
                if (threadFinished) {
                    if (worker->joinable()) worker->join();
                    delete worker;
                    worker = nullptr;
                    threadFinished = false;
                }
            }
        }
    }
    void render(DrawSet d) {
        doRender = true;
        curSet = d;
    }
signals:
    void done(DrawSet);
private:
    void threadRun(DrawSet d) {
        QRect &partRectBil = std::get<0>(d);
        QRect &drawRectBil = std::get<1>(d);
        const QImage &imgOrig = std::get<2>(d);
        QImage &imgBilPart = std::get<3>(d);
        imgBilPart = imgOrig.copy(partRectBil).scaled(drawRectBil.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        threadFinished = true;
        emit done(d);
    }
    bool doRender = false;
    DrawSet curSet;
    std::thread *worker = nullptr;
    bool threadFinished = false;
};

class QImageView : public QWidget {
    Q_OBJECT
public:
    enum ZKEEP {KEEP_NONE, KEEP_FIT, KEEP_FIT_FORCE, KEEP_EXPANDED, KEEP_EQUAL};
    QImageView(QWidget *parent = 0, QImage image = QImage(0, 0));
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    QImage getImage() {return view;}
    ZKEEP getKeepState() {return keep;}
signals:
    void internalDelayedLoadComplete(QImage, ZKEEP);
public slots:
    void setImage(const QImage&, ZKEEP keepStart = KEEP_FIT);
    void setImage(QString const & path, bool threadedLoad = false, ZKEEP keepStart = KEEP_FIT);
    void setKeepState(ZKEEP z) {keep = z; this->repaint();}
protected slots:
    void centerView();
private slots:
    void setImageInternal(QImage, ZKEEP keepStart);
    void handleBilinear(DrawSet);
    void hideMouse() {this->setCursor(Qt::BlankCursor);}
    void showMouse() {this->setCursor(Qt::ArrowCursor); mouseHider->start(500);}
private: //Variables
    QImage view;
    float zoom = 1.0f;
    QRect partRect;
    static float constexpr zoomMin = 0.025f;
    float zoomMax = 0.0f;
    float zoomExp = 0.0f;
    float zoomFit = 0.0f;
    ZKEEP keep;
    QPointF viewOffset = QPointF(0, 0);
    QPoint prevMPos;
    bool mouseMoving = false;
    bool paintCompletePartial = false;
    QPointF focalPoint;
    DrawSet bilinearObject;
    QBilinearWorker bilinearWorker;
    QTimer *mouseHider = new QTimer(this);
    std::thread * delayedLoader = nullptr;
private: //Methods
    void setZoom(qreal, QPointF focus = QPointF(0, 0));
    void calculateZoomLevels();
    void calculateView();
    void delayedLoad(QString const & path, ZKEEP keepStart = KEEP_FIT) {QImage nL(path); emit internalDelayedLoadComplete(nL, keepStart);}

};

#endif // QW_IMGVIEW_HPP
