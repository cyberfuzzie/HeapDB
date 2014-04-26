#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <atomic>
#include <memory>
#include <vector>

using namespace std;

template <typename K, typename V>
struct hashTableBucket;

template <typename K, typename V>
struct hashTableBucketListHead;

template <typename K, typename V>
class HashTable
{
    public:
        HashTable(unsigned int hashBits);
        HashTable(unsigned int hashBits, unsigned int (*hashFunction)(K));
        virtual ~HashTable();
        V get(const K & key);
        void put(const K & key, const V & value);
        bool putIfNotExists(const K & key, const V & value);
        void remove(const K & key);
    private:
        unsigned int hashBits;
        unsigned int (*hashFunction)(K);
        unique_ptr<hashTableBucket<K,V>[]> buckets;
        unsigned int countBucketElements(unsigned int bucket);
        bool findKeyWait(const K & key, struct hashTableBucketListHead<K,V>* lastHeadPtr = nullptr, unsigned int lastListPos = 0);
        void moveBucketContentsBack(unsigned int bucket);
        bool putNew(const K & key, const V & value);
        bool resizeBucket(unsigned int bucket, struct hashTableBucketListHead<K,V>* headPtr);
        void unlinkBucketList(int bucket, struct hashTableBucketListHead<K,V>* headPtr);
};

// include the template implementation
#include "hashtable.cpp"

#endif // HASHTABLE_H
