#ifndef BUFFERFRAME_H
#define BUFFERFRAME_H

#include <atomic>
#include <memory>

using namespace std;

class BufferFrame
{
    public:
        /**
         * @brief BufferFrame Creates a new BufferFrameInternal.
         *        It is write-locked to allow initialization.
         */
        BufferFrame();
        virtual ~BufferFrame();
        void* getData();
        uint64_t getFrameId();
        void setFrameId(uint64_t frameId);
        void rdlock();
        void wrlock();
        bool trywrlock();
        void unlock();
        uint64_t getMappedPageId();
        void mapPage(uint64_t pageId, uint64_t pageSize);
        void writePage(uint64_t pageSize);

        atomic<uint64_t> refCount;
        volatile atomic<bool> writePossible;

    protected:
        unique_ptr<char[]> data;
        uint64_t frameId;
        atomic<uint64_t> pageId;
        pthread_rwlock_t lock;
        inline void calculateFilename(uint64_t pageId, char* buffer, size_t bufLen);
        inline uint64_t extractActualPageId(uint64_t pageId);
        inline uint64_t extractSegmentId(uint64_t pageId);
};

#endif // BUFFERFRAME_H
