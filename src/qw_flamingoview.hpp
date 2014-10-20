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
    QFlamingoView(QWidget *parent = 0);
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void processArgumentList(QFileInfoArgumentList fi);
    void clear() {qhlib.clear();}
    void abortLoad() {abort = true;} //Does nothing if arguments are not being processed.
signals:
    void statusUpdate(QString);
    void imageChanged(QString);
public slots:
    void Next();
    void Prev();
    void Rand();
private slots:
    void handleQHLIStatus(QString);
    void flamSetImage(QImage);
private:
    void internalProcessArgs(QFileInfoArgumentList fi);
    void recurseThroughQDir(QFileInfoList&, const QDir&, int iter = std::numeric_limits<int>::max());
    QHotLoadImageBay qhlib;
    bool activeState = false;
    bool abort = false; //Used for aborting long argument load sequences (in the case of recursive loading.)
};

#endif // QO_FLAMINGOVIEWMGR_HPP
