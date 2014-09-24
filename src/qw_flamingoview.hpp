#ifndef QO_FLAMINGOVIEWMGR_HPP
#define QO_FLAMINGOVIEWMGR_HPP

#include <memory>

#include <QWidget>
#include <QFileInfo>

#include "qw_imgview.hpp"
#include "qo_hotloadimage.hpp"
#include "qreversiblelistiterator.hpp"
#include "qfileinfoargument.hpp"

class QFlamingoView : public QImageView {
    Q_OBJECT
public: //Methods
    QFlamingoView(QFileInfoArgumentList fi, QWidget *parent = 0);
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
signals:
    void statusUpdate(QString);
public slots:
    void Next();
    void Prev();
    void Rand();
private slots:
    void handleQHLIStatus(QString);
    void flamSetImage(QImage);
private:
    QHotLoadImageBay qhlib;
    bool activeState = false;
};

#endif // QO_FLAMINGOVIEWMGR_HPP
