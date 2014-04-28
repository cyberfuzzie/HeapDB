#include "buffermanager.h"
#include <iostream>

using namespace std;

BufferManager::BufferManager(uint64_t size)
    : mappedPages(size) {

}

BufferManager::~BufferManager() {

}

BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    // first check if already mapped
    auto mappedPage = mappedPages.get(pageId);
    bool lock = true;
    while (mappedPage == nullptr) {
        shared_ptr<BufferFrameInternal> newPage(new BufferFrameInternal());
        if (mappedPages.putIfNotExists(pageId, newPage)) {
            // now map the data
            newPage->mapPage(pageId);
            if (!exclusive) {
                newPage->unlock();
            } else {
                lock = false;
            }
            mappedPage = newPage;
        } else {
            mappedPage = mappedPages.get(pageId);
        }
    }
    if (lock) {
        if (exclusive) {
            mappedPage->wrlock();
        } else {
            mappedPage->rdlock();
        }
    }
    return *mappedPage;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
    // cast will work because BufferFrame is abstract
    BufferFrameInternal& internalFrame = static_cast<BufferFrameInternal&>(frame);
    if (isDirty) {
        // open file and write
        internalFrame.writePage();
    }
    internalFrame.unlock();
}
