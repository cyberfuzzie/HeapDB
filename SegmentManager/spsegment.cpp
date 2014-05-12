#include "spsegment.h"

#include "slottedpage.h"

#include <cassert>

using namespace std;

SPSegment::SPSegment(BufferManager& bufman, uint64_t segId, uint64_t pgCount)
    : buffermanager(bufman),
      segmentId(segId),
      pageCount(pgCount) {
}

TID SPSegment::insert(const Record& r) {

    //TODO: what to do with records that are bigger than pagesize - header - slot?

    for (uint64_t pageId = 0; pageId < pageCount; ++pageId){
        // ask all pages kindly to be the new home for our record
        BufferFrame& bf = buffermanager.fixPage(segmentId, pageId, true);
        SlottedPage sp(bf.getData(), PAGESIZE);
        if (sp.spaceAvailableFor(r)){
            uint32_t slotNr = sp.insertRecord(r);
            buffermanager.unfixPage(bf, true);
            return makeTID(pageId, slotNr);
        }else{
            buffermanager.unfixPage(bf, false);
        }
    }

    //In case no page had enough space:
    uint64_t newPageId = pageCount;
    BufferFrame& bf = buffermanager.fixPage(segmentId, newPageId, true);
    SlottedPage sp(bf.getData(), PAGESIZE);
    sp.getHeader().dataStart = PAGESIZE;
    sp.getHeader().slotCount = 0;
    uint32_t slotNr = sp.insertRecord(r);
    buffermanager.unfixPage(bf, true);

    pageCount++;

    return makeTID(newPageId, slotNr);
}

bool SPSegment::remove(TID tid) {

}

Record SPSegment::lookup(TID tid) {
    uint64_t pageId = getPageId(tid);
    uint64_t slotId = getSlotId(tid);
    BufferFrame& bf = buffermanager.fixPage(segmentId, pageId, false);
    SlottedPage sp(bf.getData(), PAGESIZE);
    Slot s = sp.lookup(slotId);
    if (s.isRedirect()) {
        pageId = getPageId(s.getRedirect());
        slotId = getSlotId(s.getRedirect());
        //TODO: should this locking be the other way around?
        //Somebody might change the redirect while we follow it
        buffermanager.unfixPage(bf, false);
        buffermanager.fixPage(segmentId, pageId, false);
        sp = SlottedPage(bf.getData(), PAGESIZE);
        s = sp.lookup(slotId);
    }
    assert(s.isRedirect() == false);
    Record rec(sp.readRecord(s));
    buffermanager.unfixPage(bf, false);
    return rec;
}



bool SPSegment::update(TID tid, const Record& r) {

}


Slot SPSegment::getSlot(TID tid) {

}

inline uint64_t SPSegment::getPageId(TID tid){
    return (tid >> 24);
}

inline uint64_t SPSegment::getSlotId(TID tid){
    return (tid & 0xffffff);
}

inline TID SPSegment::makeTID(uint64_t pageID, uint64_t slotNr){
    return ((pageID << 24) | slotNr & 0xffffff);
}
