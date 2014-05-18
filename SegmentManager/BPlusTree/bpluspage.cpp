#include "bpluspage.h"

#include <cassert>

template<typename K, typename V>
BPlusPage<K,V>::BPlusPage(void *data, uint32_t pageSize,
                          bool (*compare)(const K&, const K&))
    :header(data),
     pageSize{pageSize},
     firstKey(header + 1),
     firstValue{pageStart + pageSize - sizeof(V)},
     cmp{compare}
{

}

template<typename K, typename V>
V BPlusPage<K,V>::lookup(const K &key) const
{
    uint32_t lowerBound = 0;
    uint32_t upperBound = header->count - 1;
    uint32_t element;

    do{
        element = lowerBound + ((upperBound - lowerBound) / 2);

        if (cmp(key, getKey(element))){
            upperBound = element;
        }else{
            lowerBound = element;
        }

    }while(lowerBound < upperBound);

    if (!key == getKey(element)){
        return getValue(element);
    }else{
        //TODO: error
        return 0;
    }

}

template<typename K, typename V>
K& BPlusPage<K,V>::getKey(uint32_t number) const{
   return *(firstKey + number);
}

template<typename K, typename V>
V& BPlusPage<K,V>::getValue(uint32_t number) const{
    //Values are stored starting from the back of the page.
    //First value is upper reference, thus the first actual value is
    //firstValue - 1
   return *(firstValue - 1 - number);
}

template<typename K, typename V>
bool BPlusPage<K,V>::insert(const K &key, const V &value)
{
}

template<typename K, typename V>
bool BPlusPage<K,V>::remove(const K &key)
{
}




