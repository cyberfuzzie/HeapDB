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

typedef uint64_t SlotID;
typedef uint64_t PageID;


class SPSegment {
    public:
        SPSegment(SegmentManager& segman, BufferManager& bufman, uint64_t segId, uint64_t pgcount);
        // Move Constructor
        SPSegment(SPSegment&& other);
        // Copy Constructor: deleted
        SPSegment(SPSegment& other) = delete;

        // Assignment Operator: deleted
        SPSegment& operator=(SPSegment& rhs) = delete;

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
        SlotID getSlotId(TID tid) const;
        PageID getPageId(TID tid) const;
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
