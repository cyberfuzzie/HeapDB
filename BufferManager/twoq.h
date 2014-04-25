#ifndef TWOQ_H
#define TWOQ_H

#include <cstdint>

template<typename T>
class TwoQ
{
public:
    TwoQ();

    /**
     * @brief promote Tells strategy that T has been accessed
     * @param bufferFrame
     */
    void promote(T element);

    /**
     * @brief findPageToUnfixFor Finds a T that should be unfixed and used for the given T.
     * @param element
     * @return the pageID to unfix
     */
    T findPageToUnfixFor(T element);

    /**
     * @brief unfixed Notifies the strategy that a T has been removed
     * @param T
     */
    void unfixed(T element);
};

#endif // TWOQ_H
