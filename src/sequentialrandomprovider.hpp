#ifndef SEQUENTIALRANDOMPROVIDER_HPP
#define SEQUENTIALRANDOMPROVIDER_HPP

#include <QQueue>

class SequentialRandomProvider : protected QQueue<int> {
public:
    SequentialRandomProvider(int size, int maxValue) : QQueue<int>(), keepSize(size) { Reset(maxValue); }
    virtual ~SequentialRandomProvider() {}
    void Reset(int maxValue);
    int Next();
    int NextPeek(int skip);
protected:
    int keepSize, maxValue;
    void EnqueueNew();
};

#endif // SEQUENTIALRANDOMPROVIDER_HPP
