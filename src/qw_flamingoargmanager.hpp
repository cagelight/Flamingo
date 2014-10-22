#ifndef QW_FLAMINGOARGMANAGER_HPP
#define QW_FLAMINGOARGMANAGER_HPP

#include "qfileinfoargument.hpp"
#include <QtWidgets>

class QImageView;

class QFlamingoArgManager : public QDialog {
    Q_OBJECT
public:
    QFlamingoArgManager(QFileInfoArgumentList, QWidget *parent = 0);
    static QFileInfoArgumentList QFIALFromFLID(QString const & file);
    static bool fileIsFLID(QString const & file);
signals:
    void updateFinalized(QFileInfoArgumentList);
protected slots:
    void setupViews();
    void handleDirItemDoubleClicked(QTreeWidgetItem *, int);
    void handleArgItemDoubleClicked(QListWidgetItem *);
    void handleDirItemClicked(QTreeWidgetItem *, int);
    void handleArgItemClicked(QListWidgetItem *);
    void addNewArgFile();
    void addNewArgDir();
    void finalizeUpdates();
    void discardUpdates();
    void saveToFile();
    void loadFromFile();
protected:
    void showEvent(QShowEvent * QSE) {argsPrev = args; this->setupViews(); QDialog::showEvent(QSE);}
    void hideEvent(QHideEvent * QHE) {this->clear(); QDialog::hideEvent(QHE);}
    void closeEvent(QCloseEvent * QCE) {this->clear(); QDialog::closeEvent(QCE);}
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent *);
    void keyPressEvent(QKeyEvent *);
private:
    class QArgDirTreeWidgetItem : public QTreeWidgetItem {
    public:
        QArgDirTreeWidgetItem(QTreeWidget * view, QString name, QFileInfoArgument & qfia) : QTreeWidgetItem(view, QStringList(name)), arg(qfia) {}
        QArgDirTreeWidgetItem(QTreeWidgetItem * parent, QString name, QFileInfoArgument & qfia) : QTreeWidgetItem(parent, QStringList(name)), arg(qfia) {}
        virtual ~QArgDirTreeWidgetItem() {}
        QFileInfoArgument & arg;
    };
    class QArgFileListWidgetItem : public QListWidgetItem {
    public:
        QArgFileListWidgetItem(QListWidget * view, QString name, QFileInfoArgument & qfia) : QListWidgetItem(name, view), arg(qfia) {}
        ~QArgFileListWidgetItem() {}
        QFileInfoArgument & arg;
    };

    typedef std::pair<QFileInfoArgument &, QStringList> QFIASplit;
    void clear();
    QIcon getQFIAIcon(QFileInfoArgument &);
    QGridLayout * layout = new QGridLayout(this);
    QTreeWidget * dirView = new QTreeWidget(this);
    QImageView * iView = nullptr;
    QList<QTreeWidgetItem *> dirViewTopItems;
    QList<QArgDirTreeWidgetItem *> dirViewArgItems;
    QListWidget * argView = new QListWidget(this);
    QList<QListWidgetItem *> argViewItems;
    QFileInfoArgumentList argsPrev;
    QFileInfoArgumentList args;
    QUrl cDir;
    QList<QFileInfoArgument *> argsMarkedDelete;
    QMap<QFileInfoArgument *, QArgDirTreeWidgetItem *> dirViewArgMap;
    QMap<QFileInfoArgument *, QArgFileListWidgetItem *> argViewArgMap;
};

#endif // QW_FLAMINGOARGMANAGER_HPP
