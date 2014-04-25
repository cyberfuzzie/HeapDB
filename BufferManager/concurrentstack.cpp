#include "concurrentstack.h"

//TODO: What if ABA attacks?

template<typename T>
ConcurrentStack<T>::ConcurrentStack() :start{nullptr}
{

}

template<typename T>
void ConcurrentStack<T>::push(T element)
{
    Container<T>* c = new Container<T>{element};
    Container<T>* first = start.load();
    do{
        c->next = first;
    }while(start.compare_exchange_weak(first, c));
}

template<typename T>
T ConcurrentStack<T>::pop()
{
    Container<T>* first = start.load();
    Container<T>* next;
    do{
        next = first->next.load();
    }while(start.compare_exchange_weak(first, next));

    T result = first->load;
    delete first;

    return result;
}
