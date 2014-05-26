#ifndef BPLUSSEGMENT_H
#define BPLUSSEGMENT_H

#include "segment.h"
#include "tid.h"
#include "buffermanager.h"
#include "bplusheader.h"
#include "bpluspage.h"
#include "notfoundexception.h"

#include <ostream>

template<typename K, typename V>
class BPlus_iterator
         :public std::iterator<std::input_iterator_tag,
                               V, ptrdiff_t, const V*, const V&>
{
    public:
        BPlus_iterator(BufferManager& bm, uint64_t segID, PageID pageID, uint64_t offset);
        BPlus_iterator(const BPlus_iterator& other);
        const V& operator*() const;
        BPlus_iterator& operator++();
        BPlus_iterator operator++(int);
        bool operator==(const BPlus_iterator& rhs) const;
        bool operator!=(const BPlus_iterator& rhs) const;
    private:
        BufferManager& bm;
        uint64_t segID;
        PageID pageID;
        uint64_t offset;
};

template<typename K>
struct SplitResult{
    PageID siblingPageID;
    K   siblingHighestKey;
    K   pageHighestKey;
    bool hasSplit;
    BufferFrame& pageFrame;

    SplitResult(PageID sID, K sHK, K pHK, bool hS, BufferFrame& pF)
        :siblingPageID{sID},
         siblingHighestKey{sHK},
         pageHighestKey{pHK},
        hasSplit{hS},
        pageFrame(pF) {}

    SplitResult(bool hS, BufferFrame& pF)
        :hasSplit{hS},
        pageFrame(pF) {}
};


template<typename K, typename V>
class BPlusSegment :public Segment
{
public:
    BPlusSegment(bool (*comparator)(const K&, const K&),
                 BufferManager& bm,
                 SchemaManager& schemaManager,
                 uint64_t segId,
                 uint64_t pgcount,
                 uint32_t pageSize,
                 uint64_t rootPage);

    void insert(const K& key,const V& value);
    void erase(const K& key);
    V lookup(const K& key) const;
    V findGreatestKey(BufferFrame* startFrame) const;
    BPlus_iterator<K, V> lookupRange(const K& startKey) const;
    void visualize(ostream &output) const;

private:
    bool (*cmp)(const K&,const K&);
    /**
     * @brief root
     */
    /*
     * To change this field, you need to hold the X lock for the Page!
     */
    PageID root;
    BufferManager& bm;
    bool isLeaf(const void* data) const;
    /**
     * @brief fixLeafFor Returns a BufferFrame that has the page fixed that should contain key.
     * @param key
     * @param exclusive exclusive locking?
     * @throws NotFoundException in case an upper should be followed, but the page doesn't exist.
     * @return
     */
    BufferFrame* fixLeafFor(const K& key, const bool exclusive) const;
    /**
      Note: Leaves frame of pageID locked exclusively. Caller must unlock it
     * @brief insertAndSplit
     * @param key
     * @param value
     * @param pageID
     */
    SplitResult<K> insertAndSplit(const K& key, const V& value, const PageID pageID);
    void insertAndSplit(const K& key, const V& value);
    void initialize();
    SplitResult<K> splitPage(BufferFrame& frame, const bool inner);

    void visualizePage(ostream &output, const PageID pageID, vector<PageID>& pageLinks) const;
};

#include "bplussegment.cpp"

#endif // BPLUSSEGMENT_H
