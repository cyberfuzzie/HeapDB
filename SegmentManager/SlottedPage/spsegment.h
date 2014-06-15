#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include <cstdint>

// Forward declarations
class SPSegment;
class SPRecord_iterator;

#include "record.h"
#include "tid.h"
#include "segment.h"
#include "schemamanager.h"
#include "segmentmanager.h"
#include "buffermanager.h"
#include "slottedpage.h"


typedef uint64_t SlotID;

class SPRecord_iterator
         :public std::iterator<std::input_iterator_tag,
                               Record, ptrdiff_t, const Record*, const Record&>
{
    public:
        SPRecord_iterator(BufferManager& bm, SPSegment& seg);
        virtual ~SPRecord_iterator();
        const Record operator*() const;
        //TODO:
        //TID getCurrentTID() const;
        SPRecord_iterator& operator++();
        SPRecord_iterator operator++(int);
        bool operator==(const SPRecord_iterator& rhs) const;
        bool operator!=(const SPRecord_iterator& rhs) const;
        unique_ptr<SPRecord_iterator> end();
    private:
        BufferManager& bm;
        SPSegment& source;
        BufferFrame* currentFrame;
        SlottedPage* currentPage;

        uint64_t currentPageNr;
        uint64_t currentSlotNr;
};


class SPSegment :public Segment {
    public:
        SPSegment(SchemaManager& schemaManager, BufferManager& bufman, uint64_t segId, uint64_t pgcount, uint32_t pageSize);
        // Move Constructor
        SPSegment(SPSegment&& other);
        // Copy Constructor: deleted
        SPSegment(SPSegment& other) = delete;

        // Assignment Operator: deleted
        SPSegment& operator=(SPSegment& rhs) = delete;

        TID insert(const Record& r);
        bool remove(TID tid);
        Record lookup(TID tid);
        bool update(TID tid, const Record& r);

        unique_ptr<SPRecord_iterator> getRecordIterator();


    private:
        BufferManager& bm;
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
