#ifndef SEQUENTIALRANDOMPROVIDER_HPP
#define SEQUENTIALRANDOMPROVIDER_HPP

#include <QQueue>
#include <QVector>

class SequentialRandomProvider : protected QQueue<int> {
public:
    SequentialRandomProvider(int size, int maxValue) : QQueue<int>(), keepSize(size) { Reset(maxValue); }
    virtual ~SequentialRandomProvider() {}
    void Reset(int maxValue);
    int Next();
    int NextPeek(int skip);
    int Previous();
protected:
    QVector<int> rev;
    int keepSize, maxValue;
    void EnqueueNew();
};

#endif // SEQUENTIALRANDOMPROVIDER_HPP
