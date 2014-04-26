#ifndef CONCURRENTLIST_H
#define CONCURRENTLIST_H

#include <atomic>
#include <unordered_map>
#include <mutex>

using namespace std;

template<typename T>
struct Container{
    T load;
    Container<T>* previous;
    Container<T>* next;
};

/**
 *  A list that can be accessed concurrently. Synchronisation is simply done by a single mutex.
 *  All operations run in O(1).
 */
template<typename T>
class ConcurrentListSimple
{
public:
    ConcurrentListSimple();

    /**
     * @brief putTop Put the given element on top of the list.
     * @param element
     * @return True if element was inserted. False if element was already present.
     */
    bool putTop(T element);

    /**
     * @brief moveTop Moves the given element to the top of the list.
     * @param element
     * @return True if element is in list. False otherwise.
     */
    bool moveTop(T element);

    /**
     * @brief contains
     * @param element
     * @return True if element is in list. False otherwise.
     */
    bool contains(T element);

    /**
     * @brief remove Removes the element from the list.
     * @param element
     * @return True if element is found. False if element is not in list.
     */
    bool remove(T element);

    /**
     * @brief removeLast removes the last element in list
     * @return true if element could be removed, false if there was no element.
     */
    bool removeLast();

    /**
     * @brief getLast
     * @return The last element in the list
     */
    T getLast();

    /**
     * @brief getSize
     * @return number of elements in the list
     */
    uint64_t getSize();

private:
    Container<T>* first;
    Container<T>* last;
    unordered_map<T, Container<T>*> map;
    mutex m;
    atomic<uint64_t> nrElements;
};

#endif // CONCURRENTLIST_H
