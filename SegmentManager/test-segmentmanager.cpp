
#include "buffermanager.h"
#include "segmentmanager.h"
#include "spsegment.h"
#include "record.h"

int main() {
    BufferManager bm(100);
    SegmentManager sm(bm);
    try {
        sm.getSegment("test");
    } catch (int e) {
        sm.createSegment("test");
    }
    SPSegment seg = sm.getSegment("test");
    char test[12] = "Hallo Welt!";
    Record r(12, test);
    seg.insert(r);
}
