#include "buffermanager.h"
#include "emptyexception.h"

#include <cmath>

#include <iostream>

using namespace std;

BufferManager::BufferManager(uint64_t size)
    : freePages(size),
      mappedPages(ilogb(size) + 1),
      twoq(size) {

}

BufferManager::~BufferManager() {

}

BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
    // first check if already mapped
    auto mappedPage = mappedPages.get(pageId);
    bool lock = true;
    if (mappedPage != nullptr) {
        // found, should promote
        // may happen simultaneous
        twoq.promote(pageId);
    }
    while (mappedPage == nullptr) {
        bool hasFree = false;
        uint64_t freePages = this->freePages;
        while ( !hasFree && freePages > 0) {
            if (this->freePages.compare_exchange_strong(freePages, freePages-1)) {
                hasFree = true;
            } else {
                freePages = this->freePages;
            }
        }
        shared_ptr<BufferFrameInternal> newPage(nullptr);
        if (hasFree) {
            newPage = shared_ptr<BufferFrameInternal>(new BufferFrameInternal());
        } else {
            // ask 2Q
            uint64_t pageIdToUnfix;
            shared_ptr<BufferFrameInternal> pageToUnfix(nullptr);
            bool unfixing = true;
            while (unfixing) {
                try {
                    pageIdToUnfix = twoq.findElementToUnfixFor(pageId);
                    pageToUnfix = mappedPages.get(pageIdToUnfix);
                    if (pageToUnfix == nullptr) {
                        // somebody else already did this
                        continue;
                    }
                    // write-lock for unfixing
                    pageToUnfix->wrlock();
                    if (pageToUnfix->getMappedPageId() == pageIdToUnfix) {
                        // success, we are owner of the correct page
                        unfixing = false;
                    } else {
                        // page was already reused
                        pageToUnfix->unlock();
                    }
                } catch (EmptyException& ex) {
                    // ignore & retry
                }
            }
            cout << "no free page found, unfixing page " << pageIdToUnfix << endl;
            twoq.unfixed(pageIdToUnfix);
            mappedPages.remove(pageIdToUnfix);
            newPage = std::move(pageToUnfix);
            newPage->mapPage(pageId);
        }

        if (mappedPages.putIfNotExists(pageId, newPage)) {
            // now map the data
            newPage->mapPage(pageId);
            // save, have wrlock
            twoq.promote(pageId);
            if (!exclusive) {
                newPage->unlock();
            } else {
                lock = false;
            }
            mappedPage = newPage;
        } else {
            // inserting failed, delete new or unfixed page was created
            newPage->unlock();
            newPage.reset();
            this->freePages++;
            // new lookup, somebody else already mapped pageId
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
