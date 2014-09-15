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

#endif // QLISTITERATOR_1DREVERSIBLE_HPP
