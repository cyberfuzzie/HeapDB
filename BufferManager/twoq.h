#ifndef TWOQ_H
#define TWOQ_H

#include <cstdint>
#include "concurrentlist_simple.h"
#include "emptyexception.h"

template<typename T>
class TwoQ
{
public:
    TwoQ(uint64_t bufferSize);

    /**
     * @brief promote Tells strategy that T has been accessed. See concurrency notes!
     * @param bufferFrame
     */
    void promote(T element);

    /**
     * @brief findElementToUnfixFor Finds a T that should be unfixed and used for the given T.
     * @param element
     * @return the pageID to unfix
     */
    T findElementToUnfixFor(T element) throw (EmptyException);

    /**
     * @brief unfixed Notifies the strategy that a T has been removed. See concurrency notes!
     * @param T
     */
    void unfixed(T element);

private:
    uint64_t Kin;
    uint64_t Kout;

    ConcurrentListSimple<T> Am;
    ConcurrentListSimple<T> A1in;
    ConcurrentListSimple<T> A1out;


};

#include "twoq.cpp"

#endif // TWOQ_H
