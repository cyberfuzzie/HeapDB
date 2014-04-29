#ifndef CONCURRENTLIST_C
#define CONCURRENTLIST_C

#include <mutex>
#include "concurrentlist_simple.h"
#include "emptyexception.h"

template<typename T> //TODO: tune map size
ConcurrentListSimple<T>::ConcurrentListSimple()
    :first{nullptr},
    last{nullptr},
    map{new unordered_map<T, Container<T>*>},
    m{},
    nrElements{0}
{
}

template<typename T>
bool ConcurrentListSimple<T>::putTop(const T element)
{
    unique_lock<mutex> lck {m};
    //Don't allow addition to list if element is already present
    if (map->find(element) != map->end()){
        return false;
    }

    Container<T>* c = new Container<T>();

    c->next = first;
    c->load = element;

    first = c;

    if (last == nullptr){
        last = c;
    }

    if (c->next != nullptr){
        c->next->previous = c;
    }

    (*map)[element] = c;

    ++nrElements;

    return true;
}



template<typename T>
bool ConcurrentListSimple<T>::moveTop(const T element)
{
    unique_lock<mutex> lck {m};

    typename unordered_map<T, Container<T>*>::const_iterator got = map->find(element);

    if (got == map->end()){
        return false;
    }

    Container<T>* c = got->second;

    //We are done in case the element is already at the top
    if (c->previous == nullptr){
        return true;
    }

    // remove c from list
    c->previous->next = c->next;
    if (c->next != nullptr) {
        c->next->previous = c->previous;
    } else {
        last = c->previous;
    }

    // put c to front
    c->next = first;
    c->previous = nullptr;
    first->previous = c;
    first = c;

    return true;
}

template<typename T>
bool ConcurrentListSimple<T>::contains(const T element)
{
    unique_lock<mutex> lck {m};
    return map->find(element) != map->end();
}

template<typename T>
bool ConcurrentListSimple<T>::removeContainer(Container<T>* c){
    if (c == nullptr){
        return false;
    }

    //take the container out of link chain
    if (c->previous != nullptr) {
        c->previous->next = c->next;
    } else {
        first = c->next;
    }

    if (c->next != nullptr) {
        c->next->previous = c->previous;
    } else {
        last = c->previous;
    }

    //erase the container from map
    map->erase(c->load);

    delete c;

    --nrElements;

    return true;
}

template<typename T>
bool ConcurrentListSimple<T>::remove(const T element)
{
    unique_lock<mutex> lck {m};

    //identify container
    typename unordered_map<T, Container<T>*>::const_iterator got = map->find(element);

    if (got == map->end()){
        return false;
    }

    Container<T>* c = got->second;

    return removeContainer(c);
}

template<typename T>
bool ConcurrentListSimple<T>::removeLast(){
    unique_lock<mutex> lck {m};
    return removeContainer(last);
}

template<typename T>
T ConcurrentListSimple<T>::getLast() throw (EmptyException)
{
    unique_lock<mutex> lck {m};

    if (last == nullptr){
        throw EmptyException{};
    }

    return last->load;
}

template<typename T>
uint64_t ConcurrentListSimple<T>::getSize()
{
    unique_lock<mutex> lck {m};
    return nrElements;
}

#endif // CONCURRENTLIST_C
