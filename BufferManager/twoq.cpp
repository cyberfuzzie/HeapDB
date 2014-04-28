#ifndef TWOQ_C
#define TWOQ_C

#include "twoq.h"

template<typename T>
TwoQ<T>::TwoQ(uint64_t bufferSize)
    :Kin{bufferSize / 4},
      Kout{bufferSize / 2},
      Am{},
      A1in{},
      A1out{}
{
}

/**
 * Caller must assure to not call this method
 * concurrently for the same element, unless
 * element is in Am.
 *
 * In case of Buffermanager, this can be done by
 * having the BM hold an X lock on the page after
 * fixing it or when unfixing it.
 */
template<typename T>
void TwoQ<T>::promote(T element)
{
    if (Am.contains(element)){
        Am.moveTop(element);
    }else if (A1out.contains(element)){
        Am.putTop(element);
        A1out.remove(element);
    }else if (A1in.contains(element)){
        //do nothing
    }else{
        A1in.putTop(element);
    }
}

template<typename T>
T TwoQ<T>::findElementToUnfixFor(T element) throw (EmptyException)
{
    if (A1in.getSize() > Kin){
        return A1in.getLast();
    }else{
        return Am.getLast();
    }
}

/**
 * Caller must assure to not call this method
 * concurrently for the same element.
 *
 * In case of Buffermanager, this can be done by
 * having the BM hold an X lock on the page after
 * fixing it or when unfixing it.
 */
template<typename T>
void TwoQ<T>::unfixed(T element)
{
    if (A1in.contains(element)){
        A1out.putTop(element);
        A1in.remove(element);
        if (A1out.getSize() > Kout){
            A1out.removeLast();
        }
    }else if (Am.contains(element)){
        Am.remove(element);
    }
}

#endif
