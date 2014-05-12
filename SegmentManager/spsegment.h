#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include <cstdint>

#include "record.h"
#include "tid.h"
#include "buffermanager.h"
#include "slottedpage.h"


class SPSegment {
    public:
        SPSegment(BufferManager& bufman, uint64_t segId, uint64_t pgcount);
        TID insert(const Record& r);
        bool remove(TID tid);
        Record lookup(TID tid);
        bool update(TID tid, const Record& r);
    private:
        BufferManager& buffermanager;
        const uint64_t segmenId;
        uint64_t pageCount;
        Slot getSlot(TID tid);
};

#endif // SPSEGMENT_H
