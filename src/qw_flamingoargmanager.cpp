#include "qw_flamingoargmanager.hpp"
#include "qw_imgview.hpp"

QFlamingoArgManager::QFlamingoArgManager(QFileInfoArgumentList qfial, QWidget *parent) : QDialog(parent), args(qfial) {
    this->setAcceptDrops(true);
    QWidget * viewWidget = new QWidget(this);
    new QHBoxLayout(viewWidget);
    iView = new QImageView(this);
    viewWidget->layout()->addWidget(argView);
    viewWidget->layout()->addWidget(dirView);
    viewWidget->layout()->addWidget(iView);
    iView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    iView->setMinimumSize(200, 150);
    this->layout->addWidget(viewWidget);
    QWidget * buttonWidget = new QWidget(this);
    new QHBoxLayout(buttonWidget);
    QPushButton * revalButton = new QPushButton("OK", buttonWidget);
    QPushButton * closeButton = new QPushButton("Close", buttonWidget);
    QPushButton * addFButton = new QPushButton("Add Files", buttonWidget);
    QPushButton * addDButton = new QPushButton("Add Directory", buttonWidget);
    QPushButton * SaveButton = new QPushButton("Save", buttonWidget);
    QPushButton * LoadButton = new QPushButton("Load", buttonWidget);
    buttonWidget->layout()->addWidget(LoadButton);
    buttonWidget->layout()->addWidget(SaveButton);
    buttonWidget->layout()->addWidget(addDButton);
    buttonWidget->layout()->addWidget(addFButton);
    buttonWidget->layout()->addWidget(closeButton);
    buttonWidget->layout()->addWidget(revalButton);
    this->layout->addWidget(buttonWidget);
    QObject::connect(dirView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(handleDirItemDoubleClicked(QTreeWidgetItem*,int)));
    QObject::connect(argView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(handleArgItemDoubleClicked(QListWidgetItem*)));
    QObject::connect(dirView, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(handleDirItemClicked(QTreeWidgetItem*,int)));
    QObject::connect(argView, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(handleArgItemClicked(QListWidgetItem*)));
    QObject::connect(closeButton, SIGNAL(released()), this, SLOT(discardUpdates()));
    QObject::connect(revalButton, SIGNAL(released()), this, SLOT(finalizeUpdates()));
    QObject::connect(addFButton, SIGNAL(released()), this, SLOT(addNewArgFile()));
    QObject::connect(addDButton, SIGNAL(released()), this, SLOT(addNewArgDir()));
    QObject::connect(LoadButton, SIGNAL(released()), this, SLOT(loadFromFile()));
    QObject::connect(SaveButton, SIGNAL(released()), this, SLOT(saveToFile()));
    dirView->setHeaderHidden(true);
    dirView->setSelectionMode(QAbstractItemView::MultiSelection);
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
            QFileInfoArgument nArg = QFileInfoArgument(url.toLocalFile());
            bool eFlag = false;
            for (QFileInfoArgument const & qfia : args) {
                if (qfia.canonicalFilePath() == nArg.canonicalFilePath()) eFlag = true;
            }
            if (!eFlag) {
                args.append(nArg);
                this->setupViews();
            }
        }
    }
    if (o != args.length()) this->setupViews();
}

void QFlamingoArgManager::keyPressEvent(QKeyEvent *QPE) {
    if (QPE->key() == Qt::Key_Delete && dirView->selectedItems().length() > 0) {
        for (QTreeWidgetItem * pitem : dirView->selectedItems()) {
            QArgDirTreeWidgetItem * item = (QArgDirTreeWidgetItem *) pitem;
            if (dirViewArgItems.contains(item)) {
                if (item->arg.isFile()) {
                    if (argsMarkedDelete.contains(&item->arg)) {
                        argsMarkedDelete.removeAll(&item->arg);
                        QIcon i = getQFIAIcon(item->arg);
                        item->setIcon(0, i);
                        argViewArgMap[&item->arg]->setIcon(i);
                    } else {
                        argsMarkedDelete.append(&item->arg);
                        QIcon i = getQFIAIcon(item->arg);
                        item->setIcon(0, i);
                        argViewArgMap[&item->arg]->setIcon(i);
                    }
                } else {
                    if (argsMarkedDelete.contains(&item->arg)) {
                        argsMarkedDelete.removeAll(&item->arg);
                        item->arg.setRecursive(false);
                        QIcon i = getQFIAIcon(item->arg);
                        item->setIcon(0, i);
                        argViewArgMap[&item->arg]->setIcon(i);
                    } else if (item->arg.isRecursive()) {
                        argsMarkedDelete.append(&item->arg);
                        QIcon i = getQFIAIcon(item->arg);
                        item->setIcon(0, i);
                        argViewArgMap[&item->arg]->setIcon(i);
                    } else {
                        item->arg.setRecursive(true);
                        QIcon i = getQFIAIcon(item->arg);
                        item->setIcon(0, i);
                        argViewArgMap[&item->arg]->setIcon(i);
                    }
                }
            }
        }
    }
    QDialog::keyPressEvent(QPE);
}

void QFlamingoArgManager::setupViews() {
    this->clear();
    QList<QFIASplit> setupList;
    for (QFileInfoArgument & qfia : args) {
        auto nArg = new QArgFileListWidgetItem(argView, qfia.canonicalFilePath(), qfia);
        argViewItems.append(nArg);
        argViewArgMap[&qfia] = nArg;
        if (qfia.isFile()) {
            nArg->setIcon(getQFIAIcon(qfia));
            setupList.append(QFIASplit(qfia, (QDir::toNativeSeparators(qfia.canonicalFilePath())).split(QDir::separator(), QString::SkipEmptyParts)));
        } else {
            nArg->setIcon(getQFIAIcon(qfia));
            setupList.append(QFIASplit(qfia, (QDir::toNativeSeparators(qfia.canonicalFilePath())).split(QDir::separator(), QString::SkipEmptyParts)));
        }
    }
    {
        int rCount = 0;
        if (setupList.length() > 1) {
            bool cFlag = true;
            for (;cFlag;rCount++) {
                cFlag = true;
                QListIterator<QFIASplit> iter(setupList);
                if (iter.hasNext()) {
                    if (setupList.first().second.length() > rCount) {
                        QString const & first = setupList.first().second.at(rCount);
                        while (iter.hasNext()) {
                            QStringList const & sr = iter.next().second;
                            if (sr.length() > rCount) {
                                if (sr.at(rCount) != first) cFlag = false;
                            } else cFlag = false;
                        }
                    } else cFlag = false;
                } else cFlag = false;
            }
        } else if (setupList.length() > 0) {
            rCount = setupList.first().second.count();
        }
        rCount -= 2;
        if (setupList.length() > 0) {
            QStringList & t = setupList.first().second;
            QStringList n;
            for (int i = 0; i <= rCount; i++) {
                n += t.at(i);
            }
            cDir = QUrl::fromLocalFile(QDir::separator() + n.join(QDir::separator()) + QDir::separator());
        } else {
            cDir = QUrl::fromLocalFile(QDir::root().path());
        }

        for (int i = 0; i < rCount; i++) {
            QMutableListIterator<QFIASplit> iter(setupList);
            while (iter.hasNext()) {
                iter.next().second.pop_front();
            }
        }
        QMap<QString, QTreeWidgetItem *> dimap;
        QMap<QStringList const *, QString> dlmap;
        bool cFlag = true;
        for (int i = 0; cFlag; i++) {
            cFlag = false;
            for (QFIASplit const & qsl : setupList) {
                if (qsl.second.length() > i + 1) cFlag = true;
                if (i < qsl.second.length()) {
                    if (i == 0 && i == qsl.second.length() - 1) {
                        QString const & comp = qsl.second.at(i);
                        dlmap[&qsl.second] = comp;
                        if (dimap.contains(comp)) {
                            delete dimap[comp];
                            dirViewTopItems.removeAll(dimap[comp]);
                        }
                        auto t = new QArgDirTreeWidgetItem(dirView, comp, qsl.first);
                        dimap[comp] = t;
                        t->setIcon(0, getQFIAIcon(qsl.first));
                        dirViewTopItems.append(t);
                        dirViewArgItems.append(t);
                        dirViewArgMap[&qsl.first] = t;
                    }
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
                        t->setIcon(0, getQFIAIcon(t->arg));
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
        dirView->expandAll();
    }
}

void QFlamingoArgManager::clear() {
    dirViewArgMap.clear();
    argViewArgMap.clear();
    qDeleteAll(argViewItems);
    argViewItems.clear();
    qDeleteAll(dirViewTopItems);
    dirViewTopItems.clear();
    dirViewArgItems.clear();
}

void QFlamingoArgManager::handleDirItemDoubleClicked(QTreeWidgetItem * pitem, int) {
    if (dirViewArgItems.contains((QArgDirTreeWidgetItem *)pitem)) {
        auto item = (QArgDirTreeWidgetItem *)pitem;
        if (item->arg.isFile()) {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(0, i);
                argViewArgMap[&item->arg]->setIcon(i);
            } else {
                argsMarkedDelete.append(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(0, i);
                argViewArgMap[&item->arg]->setIcon(i);
            }
        } else {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                item->arg.setRecursive(false);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(0, i);
                argViewArgMap[&item->arg]->setIcon(i);
            } else if (item->arg.isRecursive()) {
                argsMarkedDelete.append(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(0, i);
                argViewArgMap[&item->arg]->setIcon(i);
            } else {
                item->arg.setRecursive(true);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(0, i);
                argViewArgMap[&item->arg]->setIcon(i);
            }
        }
    }
}

void QFlamingoArgManager::handleArgItemDoubleClicked(QListWidgetItem * pitem) {
    if (argViewItems.contains((QArgFileListWidgetItem *)pitem)) {
        auto item = (QArgFileListWidgetItem *)pitem;
        if (item->arg.isFile()) {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(i);
                dirViewArgMap[&item->arg]->setIcon(0, i);
            } else {
                argsMarkedDelete.append(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(i);
                dirViewArgMap[&item->arg]->setIcon(0, i);
            }
        } else {
            if (argsMarkedDelete.contains(&item->arg)) {
                argsMarkedDelete.removeAll(&item->arg);
                item->arg.setRecursive(false);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(i);
                dirViewArgMap[&item->arg]->setIcon(0, i);
            } else if (item->arg.isRecursive()) {
                argsMarkedDelete.append(&item->arg);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(i);
                dirViewArgMap[&item->arg]->setIcon(0, i);
            } else {
                item->arg.setRecursive(true);
                QIcon i = getQFIAIcon(item->arg);
                item->setIcon(i);
                dirViewArgMap[&item->arg]->setIcon(0, i);
            }
        }
    }
}

void QFlamingoArgManager::handleDirItemClicked(QTreeWidgetItem * pitem, int) {
    dirView->clearSelection();
    dirView->setItemSelected(pitem, true);
    if (dirViewArgItems.contains((QArgDirTreeWidgetItem *)pitem)) {
        auto item = (QArgDirTreeWidgetItem *)pitem;
        argView->clearSelection();
        argView->setItemSelected(argViewArgMap[&item->arg], true);
        if (item->arg.isFile()) {
            iView->setImage(QImage(item->arg.canonicalFilePath()));
        }
    }
}

void QFlamingoArgManager::handleArgItemClicked(QListWidgetItem * pitem) {
    if (argViewItems.contains((QArgFileListWidgetItem *)pitem)) {
        auto item = (QArgFileListWidgetItem *)pitem;
        dirView->clearSelection();
        dirView->setItemSelected(dirViewArgMap[&item->arg], true);
        if (item->arg.isFile()) {
            iView->setImage(QImage(item->arg.canonicalFilePath()));
        }
    }
}

void QFlamingoArgManager::finalizeUpdates() {
    for (QFileInfoArgument * arg : argsMarkedDelete) {
        QFileInfoArgument & t = *arg;
        args.removeAll(t);
    }
    argsMarkedDelete.clear();
    emit updateFinalized(args);
    this->close();
}

void QFlamingoArgManager::discardUpdates() {
    argsMarkedDelete.clear();
    args = argsPrev;
    this->close();
}

void QFlamingoArgManager::addNewArgFile() {
    qDebug() << cDir;
    QList<QUrl> nArgs = QFileDialog::getOpenFileUrls(this, tr("Add Files"), cDir);
    for (QUrl const & arg : nArgs) {
        if (!arg.isEmpty()) {
            QFileInfoArgument nArg = QFileInfoArgument(arg.toLocalFile());
            bool eFlag = false;
            for (QFileInfoArgument const & qfia : args) {
                if (qfia.canonicalFilePath() == nArg.canonicalFilePath()) eFlag = true;
            }
            if (!eFlag) {
                args.append(nArg);
                this->setupViews();
            }
        }
    }
    this->setupViews();
}

void QFlamingoArgManager::addNewArgDir() {
    QUrl arg = QFileDialog::getExistingDirectoryUrl(this, tr("Add Directory"), cDir);
    if (!arg.isEmpty()) {
        QFileInfoArgument nArg = QFileInfoArgument(arg.toLocalFile());
        bool eFlag = false;
        for (QFileInfoArgument const & qfia : args) {
            if (qfia.canonicalFilePath() == nArg.canonicalFilePath()) eFlag = true;
        }
        if (!eFlag) {
            args.append(nArg);
            this->setupViews();
        }
    }
}

QIcon QFlamingoArgManager::getQFIAIcon(QFileInfoArgument & arg) {
    if (arg.isFile()) {
        if (argsMarkedDelete.contains(&arg)) {
            return QIcon(":/icons/arg_file_delete.png");
        } else {
            return QIcon(":/icons/arg_file_normal.png");
        }
    } else {
        if (argsMarkedDelete.contains(&arg)) {
            return QIcon(":/icons/arg_dir_delete.png");
        } else if (arg.isRecursive()) {
            return QIcon(":/icons/arg_dir_recursive.png");
        } else {
            return QIcon(":/icons/arg_dir_single.png");
        }
    }
}

void QFlamingoArgManager::saveToFile() {
    typedef struct {bool rec; QByteArray arry;} qfiapair;
    QString path = QFileDialog::getSaveFileName(this, QString(), QDir::currentPath(), tr("Flamingo Load Instruction Data (*.flid)"));
#ifdef Q_OS_WIN32
    if (!path.endsWith(".flid")) path.append(".flid");
#endif
    if (!path.isEmpty()) {
        QList<qfiapair> wpaths;
        for (QFileInfoArgument const & arg : args)
            wpaths.append({arg.isRecursive(), arg.canonicalFilePath().toLocal8Bit()});
        int cc = 0;
        for (qfiapair const & s : wpaths)
            cc += s.arry.length();
        cc += (sizeof(int) + sizeof(bool))*args.length();
        cc += sizeof(int);
        cc += 4;
        char * bytes = new char[cc];
        QByteArray wb(bytes, cc);
        QDataStream writer(&wb, QIODevice::WriteOnly);
        int ac = args.count();
        writer.writeRawData("FLID", 4);
        writer.writeRawData((const char*)&ac, sizeof(int));
        for (qfiapair const & s : wpaths) {
            writer.writeRawData((const char*)&s.rec, sizeof(bool));
            int clen = s.arry.length();
            writer.writeRawData((const char*)&clen, sizeof(int));
            writer.writeRawData(s.arry.data(), clen);
        }
        QFile save(path);
        save.open(QIODevice::WriteOnly);
        save.write(wb);
        save.close();
        delete [] bytes;
    }
}

void QFlamingoArgManager::loadFromFile() {
    QString path = QFileDialog::getOpenFileName(this, QString(), QDir::currentPath(), tr("Flamingo Load Instruction Data (*.flid)"));
    QFileInfoArgumentList qfial = QFlamingoArgManager::QFIALFromFLID(path);
    if (!qfial.isEmpty()) {
        args.clear();
        args.append(qfial);
        this->setupViews();
    }
}

QFileInfoArgumentList QFlamingoArgManager::QFIALFromFLID(const QString &file) {
    QFileInfoArgumentList qfial;
    if (!file.isEmpty()) {
        QFile open(file);
        open.open(QIODevice::ReadOnly);
        QByteArray rb = open.readAll();
        QDataStream reader(&rb, QIODevice::ReadOnly);
        int ca = 0;
        reader.skipRawData(4);
        reader.readRawData((char*)&ca, sizeof(int));
        for (int i = 0; i < ca; i++) {
            bool rec;
            int al;
            reader.readRawData((char*)&rec, sizeof(bool));
            reader.readRawData((char*)&al, sizeof(int));
            char * ab = new char[al+1];
            ab[al] = 0x00;
            reader.readRawData(ab, al);
            qfial.append(QFileInfoArgument(QString(ab), rec));
            delete [] ab;
        }
    }
    return qfial;
}

bool QFlamingoArgManager::fileIsFLID(const QString &file) {
    if (file.isEmpty()) return false;
    QFile f(file);
    if (!f.exists()) return false;
    if (!f.open(QIODevice::ReadOnly)) return false;
    char * t4 = new char[4];
    if (f.read(t4, 4) != 4) {
        delete [] t4;
        return false;
    }
    if (t4[0] == 'F' && t4[1] == 'L' && t4[2] == 'I' && t4[3] == 'D') {
        delete [] t4;
        return true;
    } else {
        delete [] t4;
        return false;
    }
}
