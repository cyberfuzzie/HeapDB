#ifndef BPLUSHEADER_H
#define BPLUSHEADER_H

class BPlusHeader {

    bool leaf;
    uint64_t lsn;
    uint16_t count;
    /*
     *upper is not in here in order to keep header
     *length independant of template parameters.
     *Upper is instead placed at beginning of K array
     */

    bool isLeaf(){
        return leaf == 0;
    }
};

#endif // BPLUSHEADER_H
