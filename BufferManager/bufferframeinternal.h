#ifndef BUFFERFRAMEINTERNAL_H
#define BUFFERFRAMEINTERNAL_H

#include "bufferframe.h"

#include <atomic>
#include <pthread.h>

using namespace std;

class BufferFrameInternal : public BufferFrame
{
    public:
        /**
         * @brief BufferFrameInternal Creates a new BufferFrameInternal.
         *        It is write-locked to allow initialization.
         */
        BufferFrameInternal();
        void rdlock();
        void wrlock();
        void unlock();
        void mapPage(uint64_t pageId);
        void writePage();
    protected:
        atomic<uint64_t> pageId;
        atomic<bool> writePossible;
        pthread_rwlock_t lock;
};

#endif // BUFFERFRAMEINTERNAL_H
