#ifndef BPLUSSEGMENT_C
#define BPLUSSEGMENT_C

#include "bplussegment.h"


template<typename K, typename V>
BPlusSegment<K,V>::BPlusSegment(bool (*comparator)(const K&, const K&),
                                BufferManager& bufman,
                                SchemaManager& schemaManager,
                                uint64_t segId,
                                uint64_t pgcount,
                                uint32_t ps,
                                uint64_t rootPage)
    : Segment(schemaManager, segId, pgcount, ps),
      cmp(comparator),
      root{rootPage},
      bm(bufman)
{
    initialize();
}

template<typename K, typename V>
void BPlusSegment<K,V>::insert(const K &key, const V &value)
{

    //TODO: locate without X lock
    //locate page
    BufferFrame* targetPage = fixLeafFor(key, true);
    //check if insert possible
    BPlusPage<K,V> bpp(targetPage->getData(), pageSize, cmp);

    if (bpp.hasAdditionalSpace()){
    //possible: do it
        bpp.insert(key, value);

        bm.unfixPage(*targetPage, true);
    }else{
        throw 0;
    }

    //not possible: lock path

}

template<typename K, typename V>
void BPlusSegment<K,V>::erase(const K &key)
{
}

template<typename K, typename V>
V BPlusSegment<K,V>::lookup(const K &key) const
{
    BufferFrame& targetPage = *fixLeafFor(key, false);

    BPlusPage<K,V> bpp(targetPage.getData(), pageSize, cmp);
    V value = bpp.lookup(key);
    bm.unfixPage(targetPage, false);

    return value;
}

template<typename K, typename V>
BufferFrame* BPlusSegment<K,V>::fixLeafFor(const K& key, const bool exclusive) const{

    BufferFrame* pageOfKey = &bm.fixPage(segmentId, root, exclusive);

    while(!isLeaf(pageOfKey->getData())){
        BPlusPage<K, PageID> page(pageOfKey->getData(), pageSize, cmp);
        PageID nextPage = page.lookup(key);
        BufferFrame* oldBF = pageOfKey;
        pageOfKey = &bm.fixPage(segmentId, nextPage, exclusive);
        bm.unfixPage(*oldBF, false);
    }


    return pageOfKey;
}

template<typename K, typename V>
bool BPlusSegment<K,V>::isLeaf(const void* data) const{
    return static_cast<const BPlusHeader*>(data)->isLeaf();
}

template<typename K, typename V>
BPlus_iterator<V> BPlusSegment<K,V>::lookupRange(const K &startKey) const
{
}

template<typename K, typename V>
char *BPlusSegment<K,V>::visualize()
{
}

template<typename K, typename V>
void BPlusSegment<K,V>::initialize(){

    if (pageCount == 0){
        BufferFrame& frame = bm.fixPage(segmentId, 0, true);
        pageCount = 1;
        root = 0;
        scm.segmentResized(*this);
        BPlusPage<K, V> page(frame.getData(), pageSize, cmp);
        page.initialize();
        page.setLeaf(true);

        bm.unfixPage(frame, true);
    }
}

#endif
