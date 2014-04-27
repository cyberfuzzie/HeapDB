#ifndef HASHTABLE_C
#define HASHTABLE_C

#include "hashtable.h"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

// Flags

enum bucketListFlags {
    Used      = 0x00000001,
    KeyValid  = 0x00000002,
    DataValid = 0x00000004,
    Deleted   = 0x00000008,
    Moved     = 0x00000010
};

// Complete class & struct definitions

template <typename K, typename V>
struct hashTableBucketListElement {
        atomic<unsigned int> flags;
        K key;
        V value;
        hashTableBucketListElement();
};

template <typename K, typename V>
struct hashTableBucketListHead {
        vector<struct hashTableBucketListElement<K,V>> list;
        atomic<struct hashTableBucketListHead<K,V>*> next;
        atomic<uint64_t> refCount;
        hashTableBucketListHead();
        hashTableBucketListHead(int listSize);
        virtual ~hashTableBucketListHead();
};

template <typename K, typename V>
struct hashTableBucket {
        atomic<struct hashTableBucketListHead<K,V>*> next;
        hashTableBucket();
        virtual ~hashTableBucket();
};

// functions

template <typename T>
unsigned int identityHash(T input) {
    return input & 0xffffffff;
}

bool checkFlags(unsigned int flags, unsigned int pos, unsigned int neg);

// struct function implementation

template <typename K, typename V>
hashTableBucket<K,V>::hashTableBucket()
    : next(new hashTableBucketListHead<K,V>()) {}

template <typename K, typename V>
hashTableBucket<K,V>::~hashTableBucket() {
    struct hashTableBucketListHead<K,V>* oldNext = next.exchange(nullptr);
    if (oldNext != nullptr) {
        delete oldNext;
    }
}

template <typename K, typename V>
hashTableBucketListElement<K,V>::hashTableBucketListElement()
    : flags(0) {}

template <typename K, typename V>
hashTableBucketListHead<K,V>::hashTableBucketListHead()
    : hashTableBucketListHead(2) {}

template <typename K, typename V>
hashTableBucketListHead<K,V>::hashTableBucketListHead(int listSize)
    : list(listSize),
      next(nullptr),
      refCount(0) {}

template <typename K, typename V>
hashTableBucketListHead<K,V>::~hashTableBucketListHead() {
    struct hashTableBucketListHead<K,V>* oldNext = next.exchange(nullptr);
    if (oldNext != nullptr) {
        delete oldNext;
    }
}


// HashTable implementation

template <typename K, typename V>
HashTable<K,V>::HashTable(unsigned int hashBits) : HashTable(hashBits, identityHash) {}

template <typename K, typename V>
HashTable<K,V>::HashTable(unsigned int hashBits, unsigned int (*hashFunction)(K))
    : hashBits(hashBits),
      hashFunction(hashFunction),
      buckets(new struct hashTableBucket<K,V>[1 << hashBits]) {}

template <typename K, typename V>
HashTable<K,V>::~HashTable() {}

template <typename K, typename V>
unsigned int HashTable<K,V>::countBucketElements(unsigned int bucket) {
    unsigned int count = 0;
    struct hashTableBucketListHead<K,V>* headPtr = buckets[bucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            if (checkFlags(headPtr->list[i].flags, bucketListFlags::Used, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                count++;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
    return count;
}

template <typename K, typename V>
bool HashTable<K,V>::findKeyWait(const K & key, struct hashTableBucketListHead<K,V>* lastHeadPtr, unsigned int lastListPos) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = this->buckets[hashBucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            if (headPtr == lastHeadPtr && i == lastListPos) {
                headPtr->refCount--;
                return false;
            }
            // wait if reserved but no valid key
            while (checkFlags(headPtr->list[i].flags, bucketListFlags::Used, bucketListFlags::KeyValid | bucketListFlags::Moved | bucketListFlags::Deleted));
            if (headPtr->list[i].key == key && checkFlags(headPtr->list[i].flags, bucketListFlags::Used | bucketListFlags::KeyValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                headPtr->refCount--;
                return true;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
    return false;
}

template <typename K, typename V>
V HashTable<K,V>::get(const K & key) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = buckets[hashBucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            if (headPtr->list[i].key == key && checkFlags(headPtr->list[i].flags, bucketListFlags::Used | bucketListFlags::KeyValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                V& value = headPtr->list[i].value;
                headPtr->refCount--;
                return value;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
    return 0;
}

template <typename K, typename V>
void HashTable<K,V>::moveBucketContentsBack(unsigned int bucket) {
    struct hashTableBucketListHead<K,V>* headPtr = this->buckets[bucket].next.load();
    headPtr->refCount++;
    while (headPtr->next.load() != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            while (checkFlags(headPtr->list[i].flags, bucketListFlags::Used, bucketListFlags::Moved)) {
                unsigned int flags = headPtr->list[i].flags;
                if (checkFlags(flags, bucketListFlags::Used | bucketListFlags::KeyValid | bucketListFlags::DataValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                    if (headPtr->list[i].flags.compare_exchange_strong(flags, bucketListFlags::Used | bucketListFlags::KeyValid | bucketListFlags::DataValid | bucketListFlags::Moved)) {
                        // now actually move it
                        putNew(headPtr->list[i].key, headPtr->list[i].value);
                    }
                }
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
}

template <typename K, typename V>
void HashTable<K,V>::put(const K & key, const V & value) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = this->buckets[hashBucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            if (headPtr->list[i].key == key && checkFlags(headPtr->list[i].flags, bucketListFlags::Used | bucketListFlags::KeyValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                headPtr->list[i].value = value;
                headPtr->refCount--;
                return;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
    putNew(key, value);
}

template <typename K, typename V>
bool HashTable<K,V>::putIfNotExists(const K & key, const V & value) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = this->buckets[hashBucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            if (headPtr->list[i].key == key && checkFlags(headPtr->list[i].flags, bucketListFlags::Used | bucketListFlags::KeyValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                headPtr->refCount--;
                return false;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
    return putNew(key, value);
}

template <typename K, typename V>
bool HashTable<K,V>::putNew(const K & key, const V & value) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = buckets[hashBucket].next.load();
    struct hashTableBucketListHead<K,V>* nextHeadPtr = headPtr->next.load();
    while (nextHeadPtr != nullptr) {
        headPtr = nextHeadPtr;
        nextHeadPtr = nextHeadPtr->next.load();
    }
    headPtr->refCount++;
    for (unsigned int i = 0; i < headPtr->list.size(); i++) {
        unsigned int flags = 0;
        if ( headPtr->list[i].flags.compare_exchange_strong(flags, bucketListFlags::Used) ) {
            flags |= bucketListFlags::Used;
            headPtr->list[i].key = key;
            headPtr->list[i].flags.compare_exchange_strong(flags, bucketListFlags::Used | bucketListFlags::KeyValid);
            flags |= bucketListFlags::KeyValid;
            if (findKeyWait(key, headPtr, i)) {
                headPtr->list[i].flags.compare_exchange_strong(flags, bucketListFlags::Used | bucketListFlags::Deleted);
                headPtr->refCount--;
                return false;
            } else {
                headPtr->list[i].value = value;
                headPtr->list[i].flags.compare_exchange_strong(flags, bucketListFlags::Used | bucketListFlags::KeyValid | bucketListFlags::DataValid);
                headPtr->refCount--;
                return true;
            }
        }
    }

    // no empty place found, do resize
    bool resized = resizeBucket(hashBucket, headPtr);
    bool putSuccess = putNew(key, value);
    if (resized) {
        moveBucketContentsBack(hashBucket);
        unlinkBucketList(hashBucket, headPtr);
        // wait until nobody uses it
        while (headPtr->refCount > 1);
        headPtr->next.store(nullptr);
        delete headPtr;
    }
    return putSuccess;
}

template <typename K, typename V>
bool HashTable<K,V>::resizeBucket(unsigned int bucket, struct hashTableBucketListHead<K,V>* headPtr) {
    headPtr->refCount++;
    struct hashTableBucketListHead<K,V>* nextHeadPtr = headPtr->next.load();
    if (nextHeadPtr != nullptr) {
        headPtr->refCount--;
        return false;
    }
    unsigned int newListSize = countBucketElements(bucket);
    if (newListSize * 2 > headPtr->list.size()) {
        newListSize = headPtr->list.size() * 2;
    }
    struct hashTableBucketListHead<K,V>* newHeadPtr  = new hashTableBucketListHead<K,V>(newListSize);
    if (!headPtr->next.compare_exchange_strong(nextHeadPtr, newHeadPtr)) {
        delete newHeadPtr;
        headPtr->refCount--;
        return false;
    }
    headPtr->refCount--;
    return true;
}

template <typename K, typename V>
void HashTable<K,V>::remove(const K & key) {
    unsigned int hashBucket = hashFunction(key) & ((1 << hashBits) - 1);
    struct hashTableBucketListHead<K,V>* headPtr = this->buckets[hashBucket].next.load();
    headPtr->refCount++;
    while (headPtr != nullptr) {
        for (unsigned int i = 0; i < headPtr->list.size(); i++) {
            unsigned int flags = headPtr->list[i].flags;
            while (headPtr->list[i].key == key && checkFlags(flags, bucketListFlags::Used | bucketListFlags::KeyValid, bucketListFlags::Moved | bucketListFlags::Deleted)) {
                headPtr->list[i].flags.compare_exchange_strong(flags, flags | bucketListFlags::Deleted);
                flags = headPtr->list[i].flags;
            }
        }
        headPtr->refCount--;
        headPtr = headPtr->next.load();
        if (headPtr != nullptr) {
            headPtr->refCount++;
        }
    }
}

template <typename K, typename V>
void HashTable<K,V>::unlinkBucketList(int bucket, struct hashTableBucketListHead<K,V>* unlinkHeadPtr) {
    struct hashTableBucketListHead<K,V>* headPtr = buckets[bucket].next.load();
    if ( !buckets[bucket].next.compare_exchange_strong(unlinkHeadPtr, unlinkHeadPtr->next.load()) ) {
        while (headPtr != nullptr && !headPtr->next.compare_exchange_strong(unlinkHeadPtr, unlinkHeadPtr->next.load())) {
            headPtr = headPtr->next.load();
        }
    }
}

#endif // HASHTABLE_C
