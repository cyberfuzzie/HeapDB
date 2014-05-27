
#include "buffermanager.h"


#include <cassert>

BufferManager::BufferManager(uint64_t pageSize, uint64_t onlineFrameCount)
    : frames(new BufferFrame[onlineFrameCount]),
      pageSize(pageSize),
      mappedPages(frames.get()),
      twoq{onlineFrameCount}
{
    freeFrames.reserve(onlineFrameCount);
    for (uint64_t i = 0; i < onlineFrameCount; i++) {
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
            //tell TwoQ that frame is used
            twoq.promote(&(frames[frameId-1]));
            assert(frames[frameId-1].writePossible == exclusive);
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

                //get a frame that we can reclaim
                BufferFrame* reclaimCandidate = twoq.getFrameForReclaim();

                frameId = reclaimCandidate->getFrameId();
                assert(frameId > 0);
                result = mappedPages.removeFrameLocked(*reclaimCandidate);
                if (result) {
                    frames[frameId-1].refCount++;
                    // lock already set
                } else {
                    //in case the frame couldn't be removed from the hashtable:
                    //tell twoq that we were not able to reclaim
                    twoq.promote(reclaimCandidate);
                    continue;
                }
            }


            frames[frameId-1].mapPage(pageId, pageSize);

            //tell replaceMgr that page with frame is in use
            twoq.promote(&frames[frameId-1]);

            result = mappedPages.insertFrameIfNotExists(frames[frameId-1]);
            if (result) {
                if (!exclusive) {
                    frames[frameId-1].unlock();
                    frames[frameId-1].rdlock();
                }

                if (frames[frameId-1].getMappedPageId() == pageId) {
                    assert(frames[frameId-1].writePossible == exclusive);
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

BufferFrame& BufferManager::fixPage(uint64_t segmentId, uint64_t pageId, bool exclusive) {
    uint64_t combinedPageId = (segmentId << 40) | (pageId & 0xffffffffff);
    return fixPage(combinedPageId, exclusive);
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    // give up the lock on the BufferFrame
    // decrease the reference counter
    // if it reaches 0 mark the page as reclaimable

    // write if modified
    if (isDirty) {
        frame.writePage(pageSize);
    }

    // unlock and check refCount
    frame.unlock();
    frame.refCount--;

}

uint64_t BufferManager::getPageSize() const {
    return pageSize;
}
