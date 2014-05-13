#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include <cstdint>

// Forward declarations
class SPSegment;

#include "record.h"
#include "tid.h"
#include "segmentmanager.h"
#include "buffermanager.h"
#include "slottedpage.h"


class SPSegment {
    public:
        SPSegment(SPSegment&& other);
        SPSegment(SegmentManager& segman, BufferManager& bufman, uint64_t segId, uint64_t pgcount);

        uint64_t getSegmentId() const;
        uint64_t getPageCount() const;

        TID insert(const Record& r);
        bool remove(TID tid);
        Record lookup(TID tid);
        bool update(TID tid, const Record& r);

    private:
        SegmentManager& sm;
        BufferManager& bm;
        const uint64_t segmentId;
        uint64_t pageCount;
        Slot getSlot(TID tid);
        uint64_t getSlotId(TID tid) const;
        uint64_t getPageId(TID tid) const;
        static TID makeTID(uint64_t pageID, uint64_t slotNr);

        /**
         * @brief insert Inserts record without trying to insert it
         *  into pageIdToExlude when exclude is true.
         * @param r
         * @param exclude
         * @param pageIdToExclude
         * @return
         */
        TID insert(const Record&r, bool exclude, uint64_t pageIdToExclude);
};

#endif // SPSEGMENT_H
