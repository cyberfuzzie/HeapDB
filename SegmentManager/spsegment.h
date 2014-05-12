#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include <cstdint>

#include "Record.hpp"
#include "tid.h"



class SPSegment {
public:
    SPSegment();
    TID insert(const Record& r);
    bool remove(TID tid);
    Record lookup(TID tid);
    bool update(TID tid, const Record& r);
};

#endif // SPSEGMENT_H
