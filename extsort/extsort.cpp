#include "extsort.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <iostream>
#include <queue>
#include <vector>

#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

typedef priority_queue<shared_ptr<MergeRun>, vector<shared_ptr<MergeRun>>, MergeRunCompare> mergeQueue_t;

void externalMerge (int fdInput, int fdOutput, size_t runSize, size_t fileSize, off_t offset, size_t memSize);

MergeRun::MergeRun(int fdInput, off_t offset, size_t runSize, size_t blockSize) :
    fdInput(fdInput),
    readPos(offset),
    readCount(0),
    runSize(runSize),
    blockSize(blockSize),
    currentPosition(0),
    isEmpty(false)
{
    readBlock();
}

void MergeRun::readBlock() {
    if (readCount >= runSize) {
        isEmpty = true;
    } else {
        size_t readSize = runSize - readCount;
        if (readSize > blockSize) {
            readSize = blockSize;
        }
        currentElements.resize(readSize / sizeof(uint64_t));
        pread(fdInput, currentElements.data(), readSize, readPos);
        readPos += readSize;
        readCount += readSize;
    }
}

bool MergeRun::empty() {
    return isEmpty;
}

uint64_t MergeRun::pop() {
    uint64_t value = top();
    if (++currentPosition >= currentElements.size()) {
        readBlock();
        currentPosition = 0;
    }
    return value;
}

uint64_t MergeRun::top() {
    return currentElements[currentPosition];
}

MergeRunCompare::MergeRunCompare(const bool& revparam) :
    reverse(revparam)
{}

bool MergeRunCompare::operator() (const shared_ptr<MergeRun>& lhs, const shared_ptr<MergeRun>& rhs) {
    if (reverse) {
        return lhs->top() < rhs->top();
    } else {
        return rhs->top() < lhs->top();
    }
}

void externalSort (int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
    // the number of elements that fit into the buffer
    size_t fileSize = size * sizeof(uint64_t);
    size_t runSize = (memSize / 4096) * 4096;
    size_t runElements = runSize / sizeof(uint64_t);
    char tmpName[] = "tmpXXXXXX";
    int fdTemp = mkstemp(tmpName);
    posix_fallocate(fdTemp, 0, fileSize);

    // sort blocks of memCount elements in the input file
    for (off_t offset = 0; offset < fileSize; offset += runSize) {
        size_t curRunElements = runElements;
        if (offset + curRunElements > fileSize) {
            curRunElements = fileSize - offset;
        }
        size_t curRunSize = curRunElements * sizeof(uint64_t);
        uint64_t *buffer = static_cast<uint64_t*>(mmap(NULL, curRunSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fdInput, offset));
        sort (buffer, buffer + curRunElements);
        pwrite (fdTemp, buffer, curRunSize, offset);
        munmap(buffer, curRunSize);
    }

    externalMerge(fdTemp, fdOutput, runSize, fileSize, 0, memSize);

    // cleanup
    close(fdTemp);
    unlink(tmpName);
}

void externalMerge (int fdInput, int fdOutput, size_t runSize, size_t fileSize, off_t offset, size_t memSize) {
    size_t runCount = fileSize / runSize;
    if (runCount * runSize < fileSize) {
        runCount++;
    }

    size_t blockSize = memSize / (runCount + 1);
    if (blockSize < 4096) {
        // multiple merge runs needed
        // maximum number of runs which can be merged at once
        size_t maxRuns = memSize / 4096 - 1;
        // remaining runs after first merge phase
        size_t remainingRuns = runCount / maxRuns;
        if (remainingRuns * maxRuns < runCount) {
            remainingRuns++;
        }
        // runs to merge in each sub-merge
        size_t subRuns = runCount / remainingRuns;
        if (subRuns * remainingRuns < runCount) {
            subRuns++;
        }

        char tmpName[] = "tmpXXXXXX";
        int fdTemp = mkstemp(tmpName);
        posix_fallocate(fdTemp, 0, fileSize);
        for (size_t curRun = 0; curRun < runCount; curRun += subRuns) {
            off_t subFileOffset = offset + curRun*runSize;
            size_t subFileSize = runSize*subRuns;
            if (subFileOffset + subFileSize > fileSize) {
                subFileSize = fileSize - subFileOffset;
            }
            externalMerge (fdInput, fdTemp, runSize, subFileSize, subFileOffset, memSize);
        }

        // merge from fdTemp to fdOutput
        externalMerge (fdTemp, fdOutput, runSize*subRuns, fileSize, offset, memSize);

        // cleanup
        close(fdTemp);
        unlink(tmpName);
    } else {
        // single merge run
        blockSize = (blockSize / 4096) * 4096;
        size_t blockElements = blockSize / sizeof(uint64_t);
        size_t writeOffset = offset;
        vector<uint64_t> outputBuffer;
        mergeQueue_t mergeQueue;
        for (size_t i = 0; i < runCount; i++) {
            size_t curRunSize = runSize;
            size_t curRunOffset = offset + i * runSize;
            if (curRunOffset + curRunSize > offset + fileSize) {
                curRunSize = offset + fileSize - curRunOffset;
            }
            shared_ptr<MergeRun> curRun(new MergeRun(fdInput, curRunOffset, curRunSize, blockSize));
            mergeQueue.push(curRun);
        }

        while ( !mergeQueue.empty() ) {
            shared_ptr<MergeRun> next = mergeQueue.top();
            mergeQueue.pop();

            uint64_t nextValue = next->pop();
            outputBuffer.push_back(nextValue);

            if ( !next->empty() ) {
                mergeQueue.push(next);
            }

            if (outputBuffer.size() >= blockElements || mergeQueue.empty()) {
                size_t writeSize = outputBuffer.size()*sizeof(uint64_t);
                pwrite(fdOutput, outputBuffer.data(), writeSize, writeOffset);
                writeOffset += writeSize;
                outputBuffer.clear();
            }
        }
    }
}
