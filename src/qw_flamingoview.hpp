#ifndef QO_FLAMINGOVIEWMGR_HPP
#define QO_FLAMINGOVIEWMGR_HPP

#include <memory>

#include <QWidget>
#include <QFileInfo>

#include "qw_imgview.hpp"
#include "qo_hotloadimage.hpp"
#include "qreversiblelistiterator.hpp"

typedef std::shared_ptr<QHotLoadImage> SharedQHLI;

class QFlamingoView : public QImageView {
    Q_OBJECT
public: //Methods
    QFlamingoView(QFileInfoList fi, QWidget *parent = 0);
signals:
    void busy(QString);
    void ok();
public slots:
    void Next();
    void Prev();
private slots:
    void delayedSet(QImage);
    void loadFailed(const QHotLoadImage*);
private:
    void processLoads();
    QList<SharedQHLI> imgList;
    std::unique_ptr<QLoopingMutableListIterator<SharedQHLI>> imgListIter;
};

#endif // QO_FLAMINGOVIEWMGR_HPP
