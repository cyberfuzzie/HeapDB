#ifndef CONCURRENTSTACK_H
#define CONCURRENTSTACK_H

#include <atomic>

using namespace std;


template<typename T>
struct Container{
    //TODO: are there any loading operations required for the load
    T load;
    atomic<Container<T>*> next;
};

/**
 *  This class is a stack. All methods must be implements thread safe.
 */
template<typename T>
class ConcurrentStack
{
public:
    ConcurrentStack();

    /**
     * @brief push Put element onto the stack.
     * @param element The element to put.
     */
    void push(T element);

    /**
     * @brief pop Remove the topmost element from the stack.
     * @return the topmost element.
     */
    T pop();

private:

    atomic<Container<T>*> start;

};

#endif // CONCURRENTSTACK_H
