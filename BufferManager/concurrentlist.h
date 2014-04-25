#ifndef CONCURRENTLIST_H
#define CONCURRENTLIST_H

#include <atomic>

using namespace std;

template<typename T>
struct Container{
    //TODO: are there any loading operations required for the load?
    T load;
    bool isRemoved;
    bool isReady;
    atomic<Container<T>&> previous;
    atomic<Container<T>&> next;
};

template<typename T>
class ConcurrentList
{
public:
    ConcurrentList();

    void putTop(T element);
    bool contains(T element);
    bool remove(T container);
    T getLast();

private:
    atomic<Container<T>&> first;
    atomic<Container<T>&> last;
};

#endif // CONCURRENTLIST_H
