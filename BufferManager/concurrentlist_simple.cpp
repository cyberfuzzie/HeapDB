#include "concurrentlist_simple.h"

template<typename T>
//TODO: tune map size
ConcurrentListSimple<T>::ConcurrentListSimple()
    :map{new unordered_map<T, Container<T>*>},
    first{nullptr},
    last{nullptr},
    nrElements{0}
{
}

template<typename T>
bool ConcurrentListSimple<T>::putTop(T element)
{
    unique_lock<mutex> lck {m};
    //Don't allow addition to list if element is already present
    if (map.find(element) == map.end()){
        return false;
    }

    Container<T>* c = new Container<T>();

    c->next = first;
    c->load = element;

    first = c;

    if (last == nullptr){
        last = c;
    }

    map.insert(make_pair<T, Container<T>*>(element, c));

    ++nrElements;

    return true;
}

template<typename T>
bool ConcurrentListSimple<T>::moveTop(T element)
{
    unique_lock<mutex> lck {m};

    typename unordered_map<T, Container<T>&>::const_iterator got = map.find(element);

    if (got == map.end()){
        return false;
    }

    Container<T>* c = got->second;

    //We are done in case the element is already at the top
    if (c->previous == nullptr){
        return true;
    }

    c->previous->next = c->next;
    if (c->next != nullptr){
        c->next->previous = c->previous;
    }else{
        last = c->previous;
    }

    c->next = first;
    c->previous = nullptr;
    first = c;

    return true;
}

template<typename T>
bool ConcurrentListSimple<T>::contains(T element)
{
    unique_lock<mutex> lck {m};
    return map.find(element) != map.end();
}

template<typename T>
bool ConcurrentListSimple<T>::remove(T element)
{
    unique_lock<mutex> lck {m};

    typename unordered_map<T, Container<T>&>::const_iterator got = map.find(element);

    if (got == map.end()){
        return false;
    }

    Container<T>* c = &got->second;

    //take the container out of link chain
    if (c->previous != nullptr){
        c->previous->next = c->next;
    }else{
        first = c->next;
        c->next->previous = nullptr;
    }

    if (c->next != nullptr){
        c->next->previous = c->previous;
    }else{
        last = c->previous;
        c->previous->next = nullptr;
    }

    //erase the container from map
    map.erase(element);

    delete c;

    --nrElements;

    return true;
}

template<typename T>
T ConcurrentListSimple<T>::getLast()
{
    unique_lock<mutex> lck {m};
    return last->load;
}

template<typename T>
uint64_t ConcurrentListSimple<T>::getSize()
{
    return nrElements.load();
}
