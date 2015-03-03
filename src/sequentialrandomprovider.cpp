#include "sequentialrandomprovider.hpp"

void SequentialRandomProvider::Reset(int maxValue) {
    this->maxValue = maxValue;
    this->clear();
    for (int i = 0; i < this->keepSize; i++) {
        this->EnqueueNew();
    }
}

int SequentialRandomProvider::Next() {
    this->EnqueueNew();
    return this->dequeue();
}

int SequentialRandomProvider::NextPeek(int skip) {
    if (skip >= keepSize) skip = keepSize - 1;
    return this->at(skip);
}

void SequentialRandomProvider::EnqueueNew() {
    this->enqueue(qrand() % maxValue);
}
