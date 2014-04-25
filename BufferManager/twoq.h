#ifndef TWOQ_H
#define TWOQ_H

#include <cstdint>

class TwoQ
{
public:
    TwoQ();
    /**
     * @brief promote Tells strategy that page has been accessed
     * @param bufferFrame
     */
    void promote(uint64_t pageID);
    /**
     * @brief reclaimFor Finds a bufferframe that should be unfixed and used for the given page.
     * @param pageID
     * @return the pageID to unfix
     */
    uint64_t findPageToUnfixFor(uint64_t pageID);

    /**
     * @brief unfixed Notifies the strategy that a page has been unfixed
     * @param pageID
     */
    void unfixed(uint64_t pageID);
};

#endif // TWOQ_H
