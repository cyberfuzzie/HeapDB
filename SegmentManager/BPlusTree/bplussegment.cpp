#include "bplussegment.h"

#include "bplusheader.h"
#include "bpluspage.h"

template<typename K, typename V>
BPlusSegment<K,V>::BPlusSegment(bool (*comparator)(K, K), BufferManager& bufman, SchemaManager& schemaManager, uint64_t segId, uint64_t pgcount, uint32_t ps)
    : Segment(schemaManager, segId, pgcount, ps),
      cmp(comparator),
      bm(bufman)
{
}

template<typename K, typename V>
void BPlusSegment<K,V>::insert(const K key, const V value)
{
}

template<typename K, typename V>
void BPlusSegment<K,V>::erase(const K key)
{
}

template<typename K, typename V>
V BPlusSegment<K,V>::lookup(const K key) const
{

    BufferFrame* bf = bm.fixPage(segmentId, root, false);

    while(!isLeaf(bf->getData())){
        BPlusPage<K, PageID> page(bf->getData(), pageSize);
        PageID nextPage = page.lookup(key);
        BufferFrame* oldBF = bf;
        bf = bm.fixPage(segmentId, nextPage, false);
        bm.unfixPage(*oldBF, false);
    }

    BPlusPage<K,V> bpp(bf->getData(), pageSize);
    V value = bpp.lookup(key);
    bm.unfixPage(*bf, false);
    return value;
}

template<typename K, typename V>
bool BPlusSegment<K,V>::isLeaf(const void* data) const{
    return reinterpret_cast<BPlusHeader*>(data)->isLeaf();
}

template<typename K, typename V>
BPlus_iterator<V> BPlusSegment<K,V>::lookupRange(const K startKey) const
{
}

template<typename K, typename V>
char *BPlusSegment<K,V>::visualize()
{
}
