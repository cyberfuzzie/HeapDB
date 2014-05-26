#ifndef BPLUSPAGE_C
#define BPLUSPAGE_C

#include <cassert>

#include "bpluspage.h"

template<typename K, typename V>
BPlusPage<K,V>::BPlusPage(void* data, uint32_t pageSize,
                          bool (*compare)(const K&, const K&))
    :header(static_cast<BPlusHeader*>(data)),
     pageSize{pageSize},
     firstKey(reinterpret_cast<K*>(header + 1)),
     firstValue{reinterpret_cast<V*>(pageStart + pageSize - 2 * sizeof(V))},
    cmp(compare)
{

}

template<typename K, typename V>
void BPlusPage<K,V>::takeUpperFrom(BPlusPage<K,V>& source){

    //TODO: handle upper when copying

    uint16_t taking = source.header->count / 2;

    //copy keys from source
    memcpy(firstKey, source.firstKey + (source.header->count - taking), taking);

    //copy values from source
    memcpy(firstValue - taking + 1, source.firstValue - source.header->count + 1, taking);

    source.header->count -= taking;
}

template<typename K, typename V>
K BPlusPage<K,V>::getUpper() const
{
    return *(firstValue + 1);
}

template<typename K, typename V>
bool BPlusPage<K,V>::getUpperExists() const{
    return header->upperExists;
}

template<typename K, typename V>
void BPlusPage<K,V>::setUpperNotExists()
{
    header->upperExists = false;
}

template<typename K, typename V>
void BPlusPage<K,V>::setUpper(V &value)
{
    header->upperExists = true;
    *(firstValue + 1) = value;
}

template<typename K, typename V>
K& BPlusPage<K,V>::getHighestKey()
{
    return *(firstKey + header->count - 1);
}


template<typename K, typename V>
V BPlusPage<K,V>::lookup(const K &key) const
{
    auto position = getPositionFor(key);

    K foundKey = getKey(position);
    if (key == foundKey){
        return getValue(position);
    }else{
        //TODO: error
        throw 0;
    }
}

template<typename K, typename V>
V BPlusPage<K,V>::lookupSmallestGreaterThan(const K &key) const{
    auto position = getPositionFor(key);
    auto keyAtPos = getKey(position);

    if (cmp(keyAtPos, key)){
        //In case the position search ended at a position where
        //the stored key is "smaller" than the key to insert, move to the right
        position++;
        K& foundKey = getKey(position);
        assert(header->count == position || !cmp(foundKey, key));
    }

    if (position < header->count){
        return getValue(position);
    }else{
        throw NotFoundException();
    }
}

template<typename K, typename V>
uint32_t BPlusPage<K,V>::getPositionFor(const K& key) const{
    if (header->count == 0){
        //TODO: proper error
        return 0;
    }

    uint32_t lowerBound = 0;
    uint32_t upperBound = header->count - 1;
    uint32_t element = lowerBound + ((upperBound - lowerBound) / 2);

    while(lowerBound < upperBound){

        K& foundKey = getKey(element);
        if (key == foundKey){
            break;
        }else if (cmp(key, foundKey)){
            upperBound = element - 1;
        }else{
            lowerBound = element + 1;
        }

        element = lowerBound + ((upperBound - lowerBound) / 2);
    };

    return element;
}

template<typename K, typename V>
K& BPlusPage<K,V>::getKey(uint32_t number) const{
   return *(firstKey + number);
}

template<typename K, typename V>
V& BPlusPage<K,V>::getValue(uint32_t number) const{
   return *(firstValue - number);
}

template<typename K, typename V>
bool BPlusPage<K,V>::hasAdditionalSpace() const{
    return  pageSize
            - sizeof(BPlusHeader)
            - (header->count + 1) * sizeof(K)
            - (header->count + 2) * sizeof(V) > 0;
}

template<typename K, typename V>
bool BPlusPage<K,V>::insert(const K &key, const V &value)
{

    auto position = getPositionFor(key);
    V& valueAtPos = getValue(position);
    K& keyAtPos = getKey(position);
    if (header->count > 0 && valueAtPos == value){
        return false;
    }else if (header->count > 0 && cmp(keyAtPos, key)){
        //In case the position search ended at a position where
        //the stored key is "smaller" than the key to insert, move to the right
        position++;
        K& foundKey = getKey(position);
        assert(header->count == position || !cmp(foundKey, key));
    }

    //move keys to make space for new key
    memmove(firstKey + position + 1, firstKey + position, (header->count - position) * sizeof(K));
    *(firstKey + position) = key;

    //move values to make space for new value
    memmove(firstValue - header->count,
            firstValue - header->count + 1,
            (header->count - position) * sizeof(V));
    *(firstValue - position) = value;

    header->count++;

    return true;
}
template<typename K, typename V>
void BPlusPage<K,V>::update(const K &oldKey, const K &newKey)
{
    uint32_t position = getPositionFor(oldKey);
    assert(*(firstKey + position) == oldKey);

    *(firstKey + position) = newKey;

    if (0 < position){
        assert(cmp(*(firstKey + position - 1), newKey));
    }
    if (position < header->count - 1){
        assert(cmp(newKey, *(firstKey + position + 1)));
    }

}

template<typename K, typename V>
bool BPlusPage<K,V>::remove(const K &key)
{
}

template<typename K, typename V>
void BPlusPage<K, V>::initialize(){
    header->initialize();
}

template<typename K, typename V>
void BPlusPage<K, V>::setLeaf(const bool isLeaf)
{
    header->leaf = isLeaf;
}

#endif
