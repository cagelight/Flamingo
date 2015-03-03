#include "sequentialrandomprovider.hpp"

#define REV_MAXHISTORY 4

void SequentialRandomProvider::Reset(int maxValue) {
    this->maxValue = maxValue;
    this->clear();
    for (int i = 0; i < this->keepSize; i++) {
        this->EnqueueNew();
    }
    rev.clear();
    for (int i = 0; i < REV_MAXHISTORY; i++) {
        rev.push_back(0);
    }
}

int SequentialRandomProvider::Next() {
    this->EnqueueNew();
    if (rev.length() >= REV_MAXHISTORY) rev.pop_back();
    rev.push_front(this->dequeue());
    return rev.front();
}

int SequentialRandomProvider::NextPeek(int skip) {
    if (skip >= keepSize) skip = keepSize - 1;
    return this->at(skip);
}

int SequentialRandomProvider::Previous() {
    if (rev.length() < 2) return 0;
    rev.pop_front();
    return rev.front();
}

void SequentialRandomProvider::EnqueueNew() {
    this->enqueue(qrand() % maxValue);
}
