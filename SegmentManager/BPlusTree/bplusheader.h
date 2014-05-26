#ifndef BPLUSHEADER_H
#define BPLUSHEADER_H

class BPlusHeader {
public:
    bool leaf;
    bool upperExists;
    uint64_t lsn;
    /**
     * @brief count The number of keys and values on the page. (Upper is not included in count)
     */
    uint16_t count;
    /*
     *upper is not in here in order to keep header
     *length independant of template parameters.
     *Upper is instead placed at beginning of K array
     */

    bool isLeaf() const{
        return leaf;
    }

    void initialize() {
        leaf = true;
        upperExists = false;
        count = 0;
    }
};

#endif // BPLUSHEADER_H
