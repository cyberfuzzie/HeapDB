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
        const uint64_t segmentId;
        uint64_t pageCount;
        Slot getSlot(TID tid);
        uint64_t getSlotId(TID tid);
        uint64_t getPageId(TID tid);
        TID makeTID(uint64_t pageID, uint64_t slotNr);
};

#endif // SPSEGMENT_H
