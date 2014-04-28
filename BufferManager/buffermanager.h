#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "bufferframeinternal.h"
#include "hashtable.h"
#include "twoq.h"

#include <atomic>
#include <cstdint>
#include <memory>

using namespace std;

class BufferManager
{
    public:
        BufferManager(uint64_t size);
        virtual ~BufferManager();
        BufferFrame& fixPage(uint64_t pageId, bool exclusive);
        void unfixPage(BufferFrame& frame, bool isDirty);

    private:
        atomic<uint64_t> freePages;
        HashTable<uint64_t,shared_ptr<BufferFrameInternal>> mappedPages;
        TwoQ<uint64_t> twoq;
};

#endif // BUFFERMANAGER_H
