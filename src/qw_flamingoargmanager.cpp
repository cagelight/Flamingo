#include "qw_flamingoargmanager.hpp"

QFlamingoArgManager::QFlamingoArgManager(QFileInfoArgumentList qfial, QWidget *parent) : QDialog(parent), args(qfial) {
    this->setAcceptDrops(true);
    QWidget * viewWidget = new QWidget(this);
    new QHBoxLayout(viewWidget);
    viewWidget->layout()->addWidget(argView);
    viewWidget->layout()->addWidget(dirView);
    viewWidget->layout()->addWidget(fileView);
    this->layout->addWidget(viewWidget);
    QWidget * buttonWidget = new QWidget(this);
    new QHBoxLayout(buttonWidget);
    QPushButton * closeButton = new QPushButton("Close", buttonWidget);
    QPushButton * revalButton = new QPushButton("OK", buttonWidget);
    buttonWidget->layout()->addWidget(closeButton);
    buttonWidget->layout()->addWidget(revalButton);
    this->layout->addWidget(buttonWidget);
    QObject::connect(dirView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(handleDirItemDoubleClicked(QTreeWidgetItem*,int)));
    QObject::connect(fileView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(handleFileItemDoubleClicked(QListWidgetItem*)));
    QObject::connect(argView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(handleArgItemDoubleClicked(QListWidgetItem*)));
    QObject::connect(closeButton, SIGNAL(released()), this, SLOT(discardUpdates()));
    QObject::connect(revalButton, SIGNAL(released()), this, SLOT(finalizeUpdates()));
    this->setupViews();
}

void QFlamingoArgManager::dragEnterEvent(QDragEnterEvent* QDEE) {
    QDEE->acceptProposedAction();
}

void QFlamingoArgManager::dragMoveEvent(QDragMoveEvent* QDME) {
    QDME->acceptProposedAction();
}

void QFlamingoArgManager::dragLeaveEvent(QDragLeaveEvent* QDLE) {
    QDLE->accept();
}

void QFlamingoArgManager::dropEvent(QDropEvent *QDE) {
    QDE->accept();
    int o = args.length();
    if (QDE->mimeData()->hasUrls()) {
        for (QUrl const & url : QDE->mimeData()->urls()) {
            this->args.append(QFileInfoArgument(url.toLocalFile()));
        }
    }
    if (o != args.length()) this->setupViews();
}

void QFlamingoArgManager::setupViews() {
    this->clear();
    argsPrev = args;
    QList<QFIASplit> dirSetupList;
    for (QFileInfoArgument & qfia : args) {
        auto nArg = new QArgFileListWidgetItem(argView, qfia.canonicalFilePath(), qfia);
        argViewItems.append(nArg);
        argViewArgMap[&qfia] = nArg;
        if (qfia.isFile()) {
            nArg->setIcon(QIcon(":/icons/arg_file.png"));
            auto fArg = new QArgFileListWidgetItem(fileView, qfia.fileName(), qfia);
            fArg->setIcon(QIcon(":/icons/arg_file.png"));
            fileViewItems.append(fArg);
            fileViewArgMap[&qfia] = fArg;
        } else {
            nArg->setIcon(QIcon(nArg->arg.isRecursive() ? ":/icons/arg_recursive.png" : ":/icons/arg_single.png"));
            dirSetupList.append(QFIASplit(qfia, qfia.canonicalFilePath().split(QDir::separator(), QString::SkipEmptyParts)));
        }
    }
    QMap<QString, QTreeWidgetItem *> dimap;
    QMap<QStringList const *, QString> dlmap;
    bool cFlag = true;
    for (int i = 0; cFlag; i++) {
        cFlag = false;
        for (QFIASplit const & qsl : dirSetupList) {
            if (qsl.second.length() > i + 1) cFlag = true;
            if (i < qsl.second.length()) {
                if (i == 0) {
                    QString const & comp = qsl.second.at(i);
                    dlmap[&qsl.second] = comp;
                    if (!dimap.contains(comp)) {
                        dimap[comp] = new QTreeWidgetItem(dirView, {comp});
                        dirViewTopItems.append(dimap[comp]);
                    }
                } else if (i == qsl.second.length() - 1) {
                    QTreeWidgetItem * parent = dimap[dlmap[&qsl.second]];
                    auto t = new QArgDirTreeWidgetItem(parent, qsl.second.last(), qsl.first);
                    t->setIcon(0, QIcon(t->arg.isRecursive() ? ":/icons/arg_recursive.png" : ":/icons/arg_single.png"));
                    dirViewArgItems.append(t);
                    dirViewArgMap[&qsl.first] = t;
                } else {
                    QString const & comp = qsl.second.at(i);
                    QTreeWidgetItem * parent = dimap[dlmap[&qsl.second]];
                    dlmap[&qsl.second] = comp;
                    if (!dimap.contains(comp)) {
                        dimap[comp] = new QTreeWidgetItem(parent, {comp});
                    }
                }
            }
        }
    }
}

void QFlamingoArgManager::clear() {
    dirViewArgMap.clear();
    fileViewArgMap.clear();
    argViewArgMap.clear();
    qDeleteAll(argViewItems);
    argViewItems.clear();
    qDeleteAll(fileViewItems);
    fileViewItems.clear();
    qDeleteAll(dirViewTopItems);
    dirViewTopItems.clear();
    dirViewArgItems.clear();
    argsMarkedDelete.clear();
}

void QFlamingoArgManager::handleDirItemDoubleClicked(QTreeWidgetItem * pitem, int) {
    if (dirViewArgItems.contains((QArgDirTreeWidgetItem *)pitem)) {
        auto item = (QArgDirTreeWidgetItem *)pitem;
        if (argsMarkedDelete.contains(&item->arg)) {
            argsMarkedDelete.removeAll(&item->arg);
            item->arg.setRecursive(false);
            item->setIcon(0, QIcon(":/icons/arg_single.png"));
            argViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_single.png"));
        } else if (item->arg.isRecursive()) {
            argsMarkedDelete.append(&item->arg);
            item->setIcon(0, QIcon(":/icons/arg_delete.png"));
            argViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_delete.png"));
        } else {
            item->arg.setRecursive(true);
            item->setIcon(0, QIcon(":/icons/arg_recursive.png"));
            argViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_recursive.png"));
        }
    }
}

void QFlamingoArgManager::handleFileItemDoubleClicked(QListWidgetItem * pitem) {
    if (fileViewItems.contains((QArgFileListWidgetItem *)pitem)) {
        auto item = (QArgFileListWidgetItem *)pitem;
        if (argsMarkedDelete.contains(&item->arg)) {
            argsMarkedDelete.removeAll(&item->arg);
            item->setIcon(QIcon(":/icons/arg_file.png"));
            argViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_file.png"));
        } else {
            argsMarkedDelete.append(&item->arg);
            item->setIcon(QIcon(":/icons/arg_delete.png"));
            argViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_delete.png"));
        }
    }
}

void QFlamingoArgManager::handleArgItemDoubleClicked(QListWidgetItem * pitem) {
    if (argViewItems.contains((QArgFileListWidgetItem *)pitem)) {
        auto item = (QArgFileListWidgetItem *)pitem;
        if (item->arg.isFile()) {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                item->setIcon(QIcon(":/icons/arg_file.png"));
                fileViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_file.png"));
            } else {
                argsMarkedDelete.append(&item->arg);
                item->setIcon(QIcon(":/icons/arg_delete.png"));
                fileViewArgMap[&item->arg]->setIcon(QIcon(":/icons/arg_delete.png"));
            }
        } else {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                item->arg.setRecursive(false);
                item->setIcon(QIcon(":/icons/arg_single.png"));
                dirViewArgMap[&item->arg]->setIcon(0, QIcon(":/icons/arg_single.png"));
            } else if (item->arg.isRecursive()) {
                argsMarkedDelete.append(&item->arg);
                item->setIcon(QIcon(":/icons/arg_delete.png"));
                dirViewArgMap[&item->arg]->setIcon(0, QIcon(":/icons/arg_delete.png"));
            } else {
                item->arg.setRecursive(true);
                item->setIcon(QIcon(":/icons/arg_recursive.png"));
                dirViewArgMap[&item->arg]->setIcon(0, QIcon(":/icons/arg_recursive.png"));
            }
        }
    }
}

void QFlamingoArgManager::finalizeUpdates() {
    for (QFileInfoArgument * arg : argsMarkedDelete) {
        QFileInfoArgument & t = *arg;
        args.removeAll(t);
    }
    emit updateFinalized(args);
    this->close();
}

void QFlamingoArgManager::discardUpdates() {
    args = argsPrev;
    this->close();
}
