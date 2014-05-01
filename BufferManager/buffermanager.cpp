
#include "buffermanager.h"

#include <cassert>

BufferManager::BufferManager(uint64_t size)
    : frames(new BufferFrame[size]),
      mappedPages(frames.get()) {
    freeFrames.reserve(size);
    for (uint64_t i = 0; i < size; i++) {
        frames[i].setFrameId(i+1);
        freeFrames.push_back(i+1);
    }
}
BufferManager::~BufferManager() {

}

BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    // first try to get a lock on an already existing BufferFrame
    //   on success make sure the BufferFrame is not reclaimable and return it
    // if getting an existing BufferFrame fails claim an unused BufferFrame and load the given page
    // then try inserting it into the hashtable
    // if that fails free the newly filled BufferFrame an repeat from the beginning
    uint64_t frameId = 0;
    uint64_t loopCount = 0;
    while (true) {
        loopCount++;
        bool result = mappedPages.getFrameLocked(pageId, &frameId, exclusive);
        if (result) {
            assert(frameId > 0);
            // that was easy
            replaceMgr.removeFrame(frameId);
            bool writeCheck = frames[frameId-1].writePossible.load();
            assert(writeCheck == exclusive);
            return frames[frameId-1];
        }

        // TODO: replace with a list which has atomic removeFirst / removeLast
        {
            unique_lock<mutex> freeFramesLock(freeFramesMutex);
            if (freeFrames.size() > 0) {
                frameId = freeFrames[freeFrames.size() - 1];
                assert(frameId > 0);
                freeFrames.pop_back();
                freeFramesLock.unlock();

                frames[frameId-1].refCount++;
                frames[frameId-1].wrlock();
            } else {
                freeFramesLock.unlock();

                frameId = replaceMgr.reclaimFrame();
                assert(frameId > 0);
                result = mappedPages.removeFrameLocked(frames[frameId-1]);
                if (result) {
                    frames[frameId-1].refCount++;
                    // lock already set
                } else {
                    continue;
                }
            }

            frames[frameId-1].mapPage(pageId);
            // release the write lock for deadlock prevention
            frames[frameId-1].unlock();
            result = mappedPages.insertFrameIfNotExists(frames[frameId-1]);
            if (result) {
                if (exclusive) {
                    frames[frameId-1].wrlock();
                } else {
                    frames[frameId-1].rdlock();
                }

                if (frames[frameId-1].getMappedPageId() == pageId) {
                    bool writeCheck = frames[frameId-1].writePossible.load();
                    assert(writeCheck == exclusive);
                    return frames[frameId-1];
                }
                frames[frameId-1].unlock();
            }

            // inserting failed, or someone modified it after insertion
            frames[frameId-1].refCount--;
            if (frames[frameId-1].trywrlock()) {
                if (frames[frameId-1].refCount == 0) {
                    // have writelock and noone else has a reference
                    // safe to release the write-lock as noone is able to get a reference
                    frames[frameId-1].unlock();
                    freeFramesLock.lock();
                    freeFrames.push_back(frameId);
                    freeFramesLock.unlock();
                } else {
                    // someone wants to get a reference, let him
                    freeFramesLock.unlock();
                }
            }
        }
    }
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    // give up the lock on the BufferFrame
    // decrease the reference counter
    // if it reaches 0 mark the page as reclaimable

    // write if modified
    if (isDirty) {
        frame.writePage();
    }

    // unlock and check refCount
    frame.unlock();
    frame.refCount--;
    if (frame.refCount == 0) {
        replaceMgr.promoteFrame(frame.getFrameId());
    }
}
