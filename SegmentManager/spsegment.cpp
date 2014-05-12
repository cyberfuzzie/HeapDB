#include "spsegment.h"

#include "slottedpage.h"

#include <cassert>

using namespace std;

SPSegment::SPSegment(BufferManager& bufman, uint64_t segId, uint64_t pgCount)
    : buffermanager(bufman),
      segmenId(segId),
      pageCount(pgCount) {
}

TID SPSegment::insert(const Record& r) {

}

bool SPSegment::remove(TID tid) {

}

Record SPSegment::lookup(TID tid) {
    uint64_t pageId = (tid >> 24);
    uint64_t slotId = (tid & 0xffffff);
    BufferFrame& bf = buffermanager.fixPage(segmenId, pageId, false);
    SlottedPage sp(bf.getData(), PAGESIZE);
    Slot s = sp.lookup(slotId);
    if (s.isRedirect()) {
        pageId = (s.getRedirect() >> 24);
        slotId = (s.getRedirect() & 0xffffff);
        buffermanager.unfixPage(bf, false);
        buffermanager.fixPage(segmenId, pageId, false);
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
