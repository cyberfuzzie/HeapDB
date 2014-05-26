#ifndef BPLUSHEADER_H
#define BPLUSHEADER_H

#include "segment.h"

class BPlusHeader {
public:
    uint64_t lsn;
    PageID next;
    /**
     * @brief count The number of keys and values on the page. (Upper is not included in count)
     */
    uint16_t count;
    bool leaf;
    bool upperExists;
    /*
     *upper is not in here in order to keep header
     *length independant of template parameters.
     *Upper is instead placed at end of page
     */

    bool isLeaf() const{
        return leaf;
    }

    void initialize() {
        leaf = true;
        upperExists = false;
        count = 0;
        next = 0;
    }
};

#endif // BPLUSHEADER_H
