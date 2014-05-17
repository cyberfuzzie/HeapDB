#ifndef BPLUSSEGMENT_H
#define BPLUSSEGMENT_H

#include "segment.h"
#include "tid.h"


template<typename T>
class BPlus_iterator
         :public std::iterator<std::input_iterator_tag,
                               T, ptrdiff_t, const T*, const T&>
{
    public:
        const T& operator*() const;
        const T* operator->() const;
        BPlus__iterator& operator++();
        BPlus__iterator operator++(int);
        bool equal(BPlus__iterator const& rhs) const;
};

template<typename T>
inline bool operator==(BPlus_iterator<T> const& lhs,BPlus_iterator<T> const& rhs)
{
    return lhs.equal(rhs);
}


template<typename T>
class BPlusSegment :public Segment
{
public:
    BPlusSegment();

    void insert(const T key,const TID value);
    void erase(const T key);
    TID lookup(const T key) const;
    BPlus_iterator lookupRange(const T startKey) const;
    char* visualize();
};

#endif // BPLUSSEGMENT_H
