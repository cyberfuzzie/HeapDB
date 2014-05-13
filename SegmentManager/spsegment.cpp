#include "spsegment.h"

#include "slottedpage.h"

#include <cassert>

using namespace std;

SPSegment::SPSegment(SPSegment&& other)
    : sm(other.sm),
      bm(other.bm),
      segmentId(other.segmentId),
      pageCount(other.pageCount) {
}

SPSegment::SPSegment(SegmentManager& segman, BufferManager& bufman, uint64_t segId, uint64_t pgCount)
    : sm(segman),
      bm(bufman),
      segmentId(segId),
      pageCount(pgCount) {
}

uint64_t SPSegment::getSegmentId() const {
    return segmentId;
}

uint64_t SPSegment::getPageCount() const {
    return pageCount;
}

TID SPSegment::insert(const Record& r) {
    return insert(r, false, 0);
}

TID SPSegment::insert(const Record&r, bool exclude, uint64_t pageIdToExclude){
    //TODO: what to do with records that are bigger than pagesize - header - slot?

    for (uint64_t pageId = 0; pageId < pageCount; ++pageId) {
        if (exclude && pageIdToExclude == pageId){
            continue;
        }
        // ask all pages kindly to be the new home for our record
        BufferFrame& bf = bm.fixPage(segmentId, pageId, true);
        SlottedPage sp(bf.getData(), PAGESIZE);
        if (sp.spaceAvailableForInsert(r)) {
            uint32_t slotNr = sp.insertRecord(r);
            bm.unfixPage(bf, true);
            return makeTID(pageId, slotNr);
        } else {
            bm.unfixPage(bf, false);
        }
    }

    //In case no page had enough space:
    uint64_t newPageId = pageCount;
    BufferFrame& bf = bm.fixPage(segmentId, newPageId, true);
    SlottedPage sp(bf.getData(), PAGESIZE);
    sp.initialize();
    uint32_t slotNr = sp.insertRecord(r);
    bm.unfixPage(bf, true);

    pageCount++;
    sm.segmentResized(*this);

    return makeTID(newPageId, slotNr);
}

bool SPSegment::remove(TID tid) {
    BufferFrame& bf = bm.fixPage(segmentId, getPageId(tid), true);
    SlottedPage sp(bf.getData(), PAGESIZE);
    return sp.removeRecord(getSlotId(tid));
}

Record SPSegment::lookup(TID tid) {
    uint64_t pageId = getPageId(tid);
    uint64_t slotId = getSlotId(tid);
    BufferFrame& bf = bm.fixPage(segmentId, pageId, false);
    SlottedPage sp(bf.getData(), PAGESIZE);
    Slot s = sp.lookup(slotId);
    if (s.isRedirect()) {
        pageId = getPageId(s.getRedirect());
        slotId = getSlotId(s.getRedirect());
        //TODO: should this locking be the other way around?
        //Somebody might change the redirect while we follow it
        bm.unfixPage(bf, false);
        bm.fixPage(segmentId, pageId, false);
        sp = SlottedPage(bf.getData(), PAGESIZE);
        s = sp.lookup(slotId);
    }
    assert(s.isRedirect() == false);
    Record rec(sp.readRecord(s));
    bm.unfixPage(bf, false);
    return rec;
}



bool SPSegment::update(TID tid, const Record& r) {
    uint64_t pageId = getPageId(tid);
    uint64_t slotId = getSlotId(tid);
    BufferFrame& bf = bm.fixPage(segmentId, pageId, true);
    SlottedPage sp(bf.getData(), PAGESIZE);
    Slot s = sp.lookup(slotId);

    if (s.isRedirect()){
        uint64_t referredPageId = getPageId(tid);
        uint64_t referredSlotId = getSlotId(tid);
        //TO DISCUSS: May cause deadlock
        BufferFrame& referredbf = bm.fixPage(segmentId, referredPageId, true);
        SlottedPage referredsp(bf.getData(), PAGESIZE);

        if (sp.spaceAvailableForUpdate(slotId, r)){
            //There is enough space for record on original page
            referredsp.removeRecord(referredSlotId);
            bm.unfixPage(referredbf, true);

            bool updated = sp.updateRecord(slotId, r);
            bm.unfixPage(bf, true);
            return updated;
        }else if (referredsp.spaceAvailableForUpdate(referredSlotId, r)){
            bm.unfixPage(bf, true);

            bool updated = sp.updateRecord(slotId, r);
            bm.unfixPage(referredbf, true);
            return updated;
        }else{
            //There is enough space for record on originally referred page
            referredsp.removeRecord(referredSlotId);
            bm.unfixPage(referredbf, true);

            //insert record in new position and redirect to it
            TID newTID = insert(r, true, pageId);
            sp.redirect(slotId, newTID);
            bm.unfixPage(bf, true);
            return true;
        }

    }else{
        //In case record is not redirected
        if (sp.spaceAvailableForUpdate(slotId, r)){
            //updated records fits into old position
            bool updated = sp.updateRecord(slotId, r);
            bm.unfixPage(bf, true);
            return updated;
        }else{
            //insert record in new position and make slot a redirect
            TID newTID = insert(r, true, pageId);
            sp.redirect(slotId, newTID);
            bm.unfixPage(bf, true);
            return true;
        }
    }
}

inline uint64_t SPSegment::getPageId(TID tid) const {
    return (tid >> 24);
}

inline uint64_t SPSegment::getSlotId(TID tid) const{
    return (tid & 0xffffff);
}

inline TID SPSegment::makeTID(uint64_t pageID, uint64_t slotNr) {
    return ((pageID << 24) | (slotNr & 0xffffff));
}
