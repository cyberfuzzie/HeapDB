#include "spsegment.h"

#include "slottedpage.h"

#include <cassert>

using namespace std;

SPSegment::SPSegment(SPSegment&& other)
    : Segment(other),
      bm(other.bm)
       {
}

SPSegment::SPSegment(SchemaManager& schemaManager, BufferManager& bufman, uint64_t segId, uint64_t pgCount, uint32_t ps)
    : Segment(schemaManager, segId, pgCount, ps),
      bm(bufman)
       {
}



TID SPSegment::insert(const Record& r) {
    return insert(r, false, 0);
}

TID SPSegment::insert(const Record&r, bool exclude, uint64_t pageIdToExclude){
    //TODO: what to do with records that are bigger than pagesize - header - slot?

    for (uint64_t pageId = 0; pageId < getPageCount(); ++pageId) {
        if (exclude && pageIdToExclude == pageId){
            continue;
        }
        // ask all pages kindly to be the new home for our record
        BufferFrame& bf = bm.fixPage(getSegmentId(), pageId, true);
        SlottedPage sp(bf.getData(), pageSize);
        if (sp.spaceAvailableForInsert(r)) {
            uint32_t slotNr = sp.insertRecord(r);
            bm.unfixPage(bf, true);
            return makeTID(pageId, slotNr);
        } else {
            bm.unfixPage(bf, false);
        }
    }

    //In case no page had enough space:
    uint64_t newPageId = getPageCount();
    BufferFrame& bf = bm.fixPage(getSegmentId(), newPageId, true);
    SlottedPage sp(bf.getData(), bm.getPageSize());
    sp.initialize();
    uint32_t slotNr = sp.insertRecord(r);
    bm.unfixPage(bf, true);

    incrementPageCount();

    return makeTID(newPageId, slotNr);
}

bool SPSegment::remove(TID tid) {
    //TODO: remove also other tuple when redirected
    BufferFrame& bf = bm.fixPage(segmentId, getPageId(tid), true);
    SlottedPage sp(bf.getData(), bm.getPageSize());
    bool result = sp.removeRecord(getSlotId(tid));
    bm.unfixPage(bf, true);
    return result;
}

Record SPSegment::lookup(TID tid) {
    PageID pageId = getPageId(tid);
    SlotID slotId = getSlotId(tid);
    BufferFrame* bf = &(bm.fixPage(segmentId, pageId, false));
    SlottedPage sp(bf->getData(), bm.getPageSize());
    Slot s = sp.lookup(slotId);
    if (s.isRedirect()) {
        pageId = getPageId(s.getRedirect());
        slotId = getSlotId(s.getRedirect());
        //TODO: should this locking be the other way around?
        //Somebody might change the redirect while we follow it
        bm.unfixPage(*bf, false);
        bf = &(bm.fixPage(segmentId, pageId, false));
        sp = SlottedPage(bf->getData(), bm.getPageSize());
        s = sp.lookup(slotId);
    }
    assert(s.isRedirect() == false);
    Record rec(sp.readRecord(s));
    bm.unfixPage(*bf, false);
    return rec;
}



bool SPSegment::update(TID tid, const Record& r) {
    PageID pageId = getPageId(tid);
    SlotID slotId = getSlotId(tid);
    BufferFrame& bf = bm.fixPage(segmentId, pageId, true);
    SlottedPage sp(bf.getData(), bm.getPageSize());
    Slot s = sp.lookup(slotId);

    if (s.isRedirect()) {
        TID referredTID = s.getRedirect();
        PageID referredPageId = getPageId(referredTID);
        SlotID referredSlotId = getSlotId(referredTID);
        //TO DISCUSS: May cause deadlock
        BufferFrame& referredbf = bm.fixPage(segmentId, referredPageId, true);
        SlottedPage referredsp(referredbf.getData(), bm.getPageSize());

        if (sp.spaceAvailableForUpdate(slotId, r)) {
            //There is enough space for record on original page
            referredsp.removeRecord(referredSlotId);
            bm.unfixPage(referredbf, true);

            bool updated = sp.updateRecord(slotId, r);
            bm.unfixPage(bf, true);
            return updated;
        } else if (referredsp.spaceAvailableForUpdate(referredSlotId, r)) {
            //There is enough space for record on originally referred page
            bm.unfixPage(bf, true);

            bool updated = referredsp.updateRecord(referredSlotId, r);
            bm.unfixPage(referredbf, true);
            return updated;
        } else {
            //Other page is needed for new referred slot
            referredsp.removeRecord(referredSlotId);
            bm.unfixPage(referredbf, true);

            //insert record in new position and redirect to it
            //do not scan page pageId to prevent deadlock, we have already locked it
            TID newTID = insert(r, true, pageId);
            sp.redirect(slotId, newTID);
            Slot redirectionSlot = sp.lookup(slotId);
            assert(redirectionSlot.isRedirect());
            assert(redirectionSlot.getRedirect() == newTID);
            bm.unfixPage(bf, true);
            return true;
        }

    } else {
        //In case record is not redirected
        if (sp.spaceAvailableForUpdate(slotId, r)) {
            //updated records fits into old position
            bool updated = sp.updateRecord(slotId, r);
            bm.unfixPage(bf, true);
            return updated;
        } else {
            //insert record in new position and make slot a redirect
            TID newTID = insert(r, true, pageId);
            sp.redirect(slotId, newTID);
            Slot redirectionSlot = sp.lookup(slotId);
            assert(redirectionSlot.isRedirect());
            assert(redirectionSlot.getRedirect() == newTID);
            bm.unfixPage(bf, true);
            return true;
        }
    }
}

unique_ptr<SPRecord_iterator> SPSegment::getRecordIterator()
{
    unique_ptr<SPRecord_iterator> p(new SPRecord_iterator(bm, *this));
    return move(p);
}

inline PageID SPSegment::getPageId(TID tid) const {
    return (tid >> 24);
}

inline SlotID SPSegment::getSlotId(TID tid) const{
    return (tid & 0xffffff);
}

inline TID SPSegment::makeTID(uint64_t pageID, uint64_t slotNr) {
    return ((pageID << 24) | (slotNr & 0xffffff));
}


SPRecord_iterator::SPRecord_iterator(BufferManager &bm, SPSegment& seg)
    :
      bm(bm),
      source(seg)
{
    currentPageNr = 0;
    currentSlotNr = 0;

    currentFrame = &(bm.fixPage(source.getSegmentId(), currentPageNr, false));
    currentPage = new SlottedPage(currentFrame->getData(), bm.getPageSize());

}

SPRecord_iterator::~SPRecord_iterator()
{
    delete currentPage;
    bm.unfixPage(*currentFrame, false);
}

const Record SPRecord_iterator::operator *() const
{
    return currentPage->readRecord(currentPage->lookup(currentSlotNr));
}

bool SPRecord_iterator::operator==(const SPRecord_iterator& rhs) const{
    return rhs.currentPageNr == currentPageNr && rhs.currentSlotNr == currentSlotNr;
}

bool SPRecord_iterator::operator!=(const SPRecord_iterator& rhs) const{
    return rhs.currentPageNr != currentPageNr;
}

unique_ptr<SPRecord_iterator> SPRecord_iterator::end()
{
    unique_ptr<SPRecord_iterator> i(new SPRecord_iterator(*this));
    i->currentPageNr = source.getPageCount();
    return move(i);
}

SPRecord_iterator &SPRecord_iterator::operator ++()
{
    currentSlotNr++;
    while (currentPage->getHeader().slotCount <= currentSlotNr
           || currentPage->lookup(currentSlotNr).isRedirect()){

        currentPageNr++;
        if (currentPageNr < source.getPageCount()){
            //move to next page
            delete currentPage;
            bm.unfixPage(*currentFrame, false);

            currentFrame = &(bm.fixPage(source.getSegmentId(), currentPageNr, false));

            currentPage = new SlottedPage(currentFrame->getData(), bm.getPageSize());
            currentSlotNr = 0;
        }
    }
}



