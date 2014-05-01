#ifndef BUFFERMANAGERHASHTABLE_H
#define BUFFERMANAGERHASHTABLE_H

#include "bufferframe.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>

using namespace std;

/**
 * @brief The IdentityHash class hashes a key to itself
 */
class IdentityHash {
    public:
        size_t operator()(const uint64_t k) const;
};

class BufferManagerHashTable
{
    public:
        /**
         * @brief BufferManagerHashTable Create a new HashTable mapping pageIds on frameIds
         * @param frames a list of frames to manage
         */
        BufferManagerHashTable(BufferFrame * const frames);

        /**
         * @brief getFrameLocked Lookup the pageId in the HashTable. If found, lock it <exclusive>
         * @param pageId the pageId to lookup
         * @param frameId output parameter for the found frame
         * @param exclusive whether an exclusive or a shared lock should be aquired
         * @return true if the page was found
         *         blocks if the page is found but already has a conflicting lock
         *         false if the page is not mapped
         */
        bool getFrameLocked(uint64_t pageId, uint64_t* frameId, bool exclusive);

        /**
         * @brief insertFrameIfNotExists Put the frame in the HashTable if no entry with pageId exists
         * @param pageId the pageId to use as key
         * @param frameId the frameId to put in the HashTable
         * @return true if the frame was successfully put in the HashTable
         *         false otherwise
         */
        bool insertFrameIfNotExists(BufferFrame& frame);

        /**
         * @brief removeFrameLocked Remove the frame from the HashTable while locking the frame exclusively
         * @param pageId the pageId to look up
         * @return true if the frame could be locked exclusively
         *         false otherwise
         */
        bool removeFrameLocked(BufferFrame& frame);
    private:
        BufferFrame * const frames;
        unordered_map<uint64_t,uint64_t,IdentityHash> hashTable;
        mutex m;
};

#endif // BUFFERMANAGERHASHTABLE_H
