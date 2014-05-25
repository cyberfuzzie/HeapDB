#ifndef BPLUSSEGMENT_H
#define BPLUSSEGMENT_H

#include "segment.h"
#include "tid.h"
#include "buffermanager.h"
#include "bplusheader.h"
#include "bpluspage.h"

template<typename T>
class BPlus_iterator
         :public std::iterator<std::input_iterator_tag,
                               T, ptrdiff_t, const T*, const T&>
{
public:
        const T& operator*() const;
        const T* operator->() const;
        BPlus_iterator& operator++();
        BPlus_iterator operator++(int);
        bool equal(BPlus_iterator const& rhs) const;

};

template<typename T>
inline bool operator==(BPlus_iterator<T> const& lhs,BPlus_iterator<T> const& rhs)
{
    return lhs.equal(rhs);
}


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
    BPlus_iterator<V> lookupRange(const K& startKey) const;
    char* visualize();

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
    BufferFrame* fixLeafFor(const K& key, const bool exclusive) const;
    void initialize();
};

#include "bplussegment.cpp"

#endif // BPLUSSEGMENT_H
