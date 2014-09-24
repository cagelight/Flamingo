#ifndef QFILEINFOARGUMENT_HPP
#define QFILEINFOARGUMENT_HPP

#include <QFileInfo>
#include <QList>

class QFileInfoArgument : public QFileInfo {
public:
    QFileInfoArgument(QString path, bool recurse = false) : QFileInfo(path), recursive(recurse) {}
    bool isRecursive() const {return recursive;}
    void setRecursive(bool recurse) {recursive = recurse;}
private:
    bool recursive;
};

typedef QList<QFileInfoArgument> QFileInfoArgumentList;

#endif // QFILEINFOARGUMENT_HPP
