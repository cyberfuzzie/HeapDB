#ifndef EXTSORT_H
#define EXTSORT_H

#include <memory>
#include <queue>
#include <vector>
#include <cstdint>
#include <sys/types.h>

using namespace std;

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);

class MergeRun {
    private:
        int fdInput;
        size_t readPos;
        size_t readCount;
        size_t runSize;
        size_t blockSize;
        uint64_t *blockBuffer;
        size_t currentBlockSize;
        size_t currentBlockElements;
        size_t currentPosition;
        bool isEmpty;
        void readBlock();
    public:
        MergeRun(int fdInput, off_t offset, size_t runSize, size_t blockSize);
        bool empty();
        uint64_t pop();
        uint64_t top();
};

class MergeRunCompare;

typedef MergeRun* mergeQueueEntry_t;
typedef priority_queue<mergeQueueEntry_t, vector<mergeQueueEntry_t>, MergeRunCompare> mergeQueue_t;

class MergeRunCompare {
    private:
        bool reverse;
    public:
        MergeRunCompare(const bool& revparam = false);
        bool operator() (const mergeQueueEntry_t& lhs, const mergeQueueEntry_t& rhs);
};

#endif // EXTSORT_H
