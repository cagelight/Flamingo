#ifndef QLISTITERATOR_1DREVERSIBLE_HPP
#define QLISTITERATOR_1DREVERSIBLE_HPP

#include <QListIterator>
#include <QMutableListIterator>

template<class T>
class QReversibleListIterator {
public:
    QReversibleListIterator<T>(const QList<T>& qlist, bool reversed = false) : qlistiter(qlist), reversed(reversed) { if (reversed) qlistiter.toBack(); }
    bool canAdvance() const { return (reversed ? qlistiter.hasPrevious() : qlistiter.hasNext()); }
    const T& peekAdvance() const { return (reversed ? qlistiter.peekPrevious() : qlistiter.peekNext()); }
    const T& advance() { return (reversed ? qlistiter.previous() : qlistiter.next()); }
private:
    QListIterator<T> qlistiter;
    bool reversed;
};

template<class T>
class QReversibleMutableListIterator {
public:
    QReversibleMutableListIterator<T>(QList<T>& qlist, bool reversed = false) : qlistiter(qlist), reversed(reversed) { if (reversed) qlistiter.toBack(); }
    bool canAdvance() const { return (reversed ? qlistiter.hasPrevious() : qlistiter.hasNext()); }
    T& peekAdvance() const { return (reversed ? qlistiter.peekPrevious() : qlistiter.peekNext()); }
    T& advance() { return (reversed ? qlistiter.previous() : qlistiter.next()); }
    void remove() { qlistiter.remove(); }
private:
    QMutableListIterator<T> qlistiter;
    bool reversed;
};

template<class T>
class QLoopingListIterator {
public:
    QLoopingListIterator<T>(const QList<T>& qlist) : qlistiter(qlist) {  }
    bool canMove() const { return (qlistiter.hasPrevious() || qlistiter.hasNext()); }
    const T& peekNext() {
        const T& t = next();
        previous();
        return t;
    }
    const T& peekPrevious() {
        const T& t = previous();
        next();
        return t;
    }
    const T& next() {
        unitPos = false;
        return internalNext();
    }
    const T& previous() {
        internalPrevious();
        if (!unitPos) internalPrevious();
        unitPos = false;
        return internalNext();
    }
    const T& value() {
        return qlistiter->value();
    }
private:
    const T& internalNext() {
        if (!qlistiter.hasNext()) qlistiter.toFront();
        return qlistiter.next();
    }
    const T& internalPrevious() {
        if (!qlistiter.hasPrevious()) qlistiter.toBack();
        return qlistiter.previous();
    }
    QListIterator<T> qlistiter;
    bool unitPos = true;
};

template<class T>
class QLoopingMutableListIterator {
public:
    QLoopingMutableListIterator<T>(QList<T>& qlist) : qlistiter(qlist) {  }
    bool canMove() const { return (qlistiter.hasPrevious() || qlistiter.hasNext()); }
    T& peekNext(unsigned int jump = 0) {
        jump++;
        for (unsigned int ij = 1; ij < jump; ij++)
            next();
        T& t = next();
        for (unsigned int ij = 0; ij < jump; ij++)
            previous();
        return t;
    }
    T& peekPrevious(unsigned int jump = 0) {
        jump++;
        for (unsigned int ij = 1; ij < jump; ij++)
            previous();
        T& t = previous();
        for (unsigned int ij = 0; ij < jump; ij++)
            next();
        return t;
    }
    T& next() {
        unitPos = false;
        return internalNext();
    }
    T& previous() {
        internalPrevious();
        if (!unitPos) internalPrevious();
        unitPos = false;
        return internalNext();
    }
    T& value() {
        return qlistiter.value();
    }
    void remove() { qlistiter.remove(); }
private:
    T& internalNext() {
        if (!qlistiter.hasNext()) qlistiter.toFront();
        return qlistiter.next();
    }
    T& internalPrevious() {
        if (!qlistiter.hasPrevious()) qlistiter.toBack();
        return qlistiter.previous();
    }
    QMutableListIterator<T> qlistiter;
    bool unitPos = true;
};

#endif // QLISTITERATOR_1DREVERSIBLE_HPP
