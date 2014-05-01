#include "buffermanager.h"
#include "emptyexception.h"

#include <assert.h>
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
    shared_ptr<BufferFrameInternal> mappedPage = mappedPages.get(pageId);
    while (mappedPage != nullptr) {
        if (exclusive) {
            mappedPage->wrlock();
        } else {
            mappedPage->rdlock();
        }
        // check if we locked the wrong page
        if (mappedPage->getMappedPageId() != pageId) {
            mappedPage->unlock();
            mappedPage = mappedPages.get(pageId);
        } else {
            break;
        }
    }
    if (mappedPage != nullptr) {
        // found, should promote
        // may happen simultaneous
        twoq.promote(pageId);
        return *mappedPage;
    }

    // no luck, map the page
    bool created = false;
    bool unfixed = false;
    bool inserted = false;
    bool existed = false;
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
            created = true;
        } else {
            // ask 2Q
            uint64_t pageIdToUnfix;
            shared_ptr<BufferFrameInternal> pageToUnfix(nullptr);
            bool unfixing = true;
            while (unfixing) {
                try {
                    pageIdToUnfix = twoq.reclaim();
                    pageToUnfix = mappedPages.get(pageIdToUnfix);
                    if (pageToUnfix == nullptr) {
                        // somebody else already did this
                        //twoq.unfixed(pageIdToUnfix);
                        continue;
                    }
//                    cout << "no free page found, trying to reclaim page " << pageIdToUnfix << " at " << pageToUnfix.get() << " for page " << pageId << endl;
                    // try write-lock for unfixing
                    pageToUnfix->wrlock();
                    // test if it still in the hashtable
                    shared_ptr<BufferFrameInternal> checkPage = mappedPages.get(pageIdToUnfix);
                    if (checkPage == pageToUnfix && pageToUnfix->getMappedPageId() == pageIdToUnfix) {
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
//            cout << "unfixing page " << pageIdToUnfix << " at " << pageToUnfix.get() << endl;
            unfixed = true;
            shared_ptr<BufferFrameInternal> checkPage = mappedPages.get(pageIdToUnfix);
            assert(checkPage == pageToUnfix);
            mappedPages.remove(pageIdToUnfix);
            checkPage = mappedPages.get(pageIdToUnfix);
            assert(checkPage != pageToUnfix);
            newPage = pageToUnfix;
        }

        // register the new frame
        if (mappedPages.putIfNotExists(pageId, newPage)) {
            inserted = true;
            // now map the data
            newPage->mapPage(pageId);
            twoq.promote(pageId);
            if (!exclusive) {
                newPage->unlock();
                newPage->rdlock();
            }
            if (newPage->getMappedPageId() != pageId) {
                newPage->unlock();
            } else {
                mappedPage = newPage;
                assert(mappedPage->getMappedPageId() == pageId);
            }
        } else {
            // inserting failed, delete new or unfixed which page was created
            newPage->unlock();
            newPage.reset();
            this->freePages++;
            // new lookup, somebody else already mapped pageId
            mappedPage = mappedPages.get(pageId);
            if (mappedPage == 0) {
                continue;
            }
            if (exclusive) {
                mappedPage->wrlock();
            } else {
                mappedPage->rdlock();
            }
            shared_ptr<BufferFrameInternal> checkPage = mappedPages.get(pageId);
            if (checkPage != mappedPage || mappedPage->getMappedPageId() != pageId) {
                mappedPage->unlock();
                mappedPage = shared_ptr<BufferFrameInternal>(nullptr);
                continue;
            }
            existed = true;
            assert(mappedPage->getMappedPageId() == pageId);
        }
    }

    assert(mappedPage->getMappedPageId() == pageId);
    shared_ptr<BufferFrameInternal> checkPage = mappedPages.get(pageId);
    assert(mappedPage == checkPage);
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
