#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "bufferframe.h"
#include "buffermanagerhashtable.h"
#include "twoq.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>


using namespace std;

class BufferManager
{
    public:
        BufferManager(uint64_t size);
        virtual ~BufferManager();
        BufferFrame& fixPage(uint64_t pageId, bool exclusive);
        void unfixPage(BufferFrame& frame, bool isDirty);

    private:
        unique_ptr<BufferFrame[]> frames;
        BufferManagerHashTable mappedPages;
        vector<uint64_t> freeFrames;
        mutex freeFramesMutex;
        TwoQ twoq;
};

#endif // BUFFERMANAGER_H
