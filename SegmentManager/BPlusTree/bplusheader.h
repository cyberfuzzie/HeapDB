#ifndef BPLUSHEADER_H
#define BPLUSHEADER_H

class BPlusHeader {
public:
    bool leaf;
    uint64_t lsn;
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
        count = 0;
    }
};

#endif // BPLUSHEADER_H
