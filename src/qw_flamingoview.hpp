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
    ZKEEP getLoadKeepState() {return loadKeep;}
signals:
    void statusUpdate(QString);
    void imageChanged(QString);
public slots:
    void Next();
    void Prev();
    void Rand();
    void RandBack();
    void setKeepNone() {setKeepState(KEEP_NONE);}
    void setKeepFit() {setKeepState(KEEP_FIT);}
    void setKeepFitForce() {setKeepState(KEEP_FIT_FORCE);}
    void setKeepExpanded() {setKeepState(KEEP_EXPANDED);}
    void setKeepEqual() {setKeepState(KEEP_EQUAL);}
    void setLoadKeep(ZKEEP z) {loadKeep = z;}
    void setLoadKeepNone() {loadKeep = KEEP_NONE;}
    void setLoadKeepFit() {loadKeep = KEEP_FIT;}
    void setLoadKeepFitForce() {loadKeep = KEEP_FIT_FORCE;}
    void setLoadKeepExpanded() {loadKeep = KEEP_EXPANDED;}
    void setLoadKeepEqual() {loadKeep = KEEP_EQUAL;}
private slots:
    void handleQHLIStatus(QString);
    void flamSetImage(QImage);
private:
    void internalProcessArgs(QFileInfoArgumentList fi);
    void recurseThroughQDir(QFileInfoList&, const QDir&, int iter = std::numeric_limits<int>::max());
    QHotLoadImageBay qhlib;
    bool activeState = false;
    bool abort = false; //Used for aborting long argument load sequences (in the case of recursive loading.)
    ZKEEP loadKeep = KEEP_FIT;
};

#endif // QO_FLAMINGOVIEWMGR_HPP
