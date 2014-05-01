#include "buffermanagerhashtable.h"

#include <cassert>
#include <cstdint>
#include <mutex>

using namespace std;

size_t IdentityHash::operator()(const uint64_t k) const {
    return k;
}

BufferManagerHashTable::BufferManagerHashTable(BufferFrame * const frames)
    : frames(frames) {}

bool BufferManagerHashTable::getFrameLocked(uint64_t pageId, uint64_t* frameId, bool exclusive) {
    unique_lock<mutex> lck(m);
    // Loop until it is sure we have (not) found the correct element
    uint64_t loopCount = 0;
    while (true) {
        loopCount++;
        unordered_map<uint64_t,uint64_t,IdentityHash>::iterator it = hashTable.find(pageId);
        if (it != hashTable.end() && it->second != 0) {
            uint64_t foundFrameId = it->second;
            frames[foundFrameId-1].refCount++;
            lck.unlock();

            if (frames[foundFrameId-1].getMappedPageId() != pageId) {
                // search was successful, but someone changed the Frame before we could lock it
                // reaquire the hashTable lock, release the found Frame and repeat search
                lck.lock();
                frames[foundFrameId-1].refCount--;
                continue;
            }

            // we have a reference, nobody will change the Frame
            // wait for the lock
            if (exclusive) {
                frames[foundFrameId-1].wrlock();
            } else {
                frames[foundFrameId-1].rdlock();
            }

            // save result and return
            *frameId = foundFrameId;
            assert(frames[*frameId-1].writePossible == exclusive);
            return true;
        } else {
            // nothing found
            return false;
        }
    }
}

bool BufferManagerHashTable::insertFrameIfNotExists(BufferFrame& frame) {
    unique_lock<mutex> lck(m);
    unordered_map<uint64_t,uint64_t,IdentityHash>::iterator it = hashTable.find(frame.getMappedPageId());
    if (it != hashTable.end()) {
        // pageId already exists
        return false;
    }

    hashTable.insert(make_pair<uint64_t,uint64_t>(frame.getMappedPageId(), frame.getFrameId()));
    return true;
}

bool BufferManagerHashTable::removeFrameLocked(BufferFrame& frame) {
    unique_lock<mutex> lck(m);
    unordered_map<uint64_t,uint64_t,IdentityHash>::iterator it = hashTable.find(frame.getMappedPageId());
    if (it == hashTable.end()) {
        // pageId does not exists
        return false;
    }

    if (!frame.trywrlock()) {
        // exclusive lock failed
        return false;
    }

    // check to reduce unnecessary removal and re-insertion
    if (frame.refCount > 0) {
        // someone has a reference
        frame.unlock();
        return false;
    }

    // remove it from the hashTable
    hashTable.erase(frame.getMappedPageId());

    // test must appear after removing
    if (frame.refCount > 0) {
        // someone got a reference before erase completed
        // restore the mapping
        hashTable.insert(*it);
        frame.unlock();
        return false;
    }

    // mapping successfully removed, no one has still has a reference
    return true;
}
