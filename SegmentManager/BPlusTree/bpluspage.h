#ifndef BPLUSPAGE_H
#define BPLUSPAGE_H

#include <cstdint>
#include "bplusheader.h"


template<typename K, typename V>
class BPlusPage
{
public:
    BPlusPage(void* data, uint32_t pageSize,
              bool (*compare)(const K&, const K&));

    V lookup(const K& key) const;
    bool insert(const K& key, const V& value);
    bool remove(const K& key);

private:
    union{
        BPlusHeader* header;
        unsigned char pageStart;
    };
    uint32_t pageSize;
    K* firstKey;
    V* firstValue;
    bool (*cmp)(const K&, const K&);
    K& getKey(uint32_t number) const;
    V& getValue(uint32_t number) const;
};

#endif // BPLUSPAGE_H
