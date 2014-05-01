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
        void mapPage(uint64_t pageId);
        void writePage();

        atomic<uint64_t> refCount;
        volatile atomic<bool> writePossible;
    protected:
        unique_ptr<char[]> data;
        uint64_t frameId;
        atomic<uint64_t> pageId;
        pthread_rwlock_t lock;
};

#endif // BUFFERFRAME_H
