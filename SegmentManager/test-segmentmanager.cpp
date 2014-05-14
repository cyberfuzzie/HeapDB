
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
    srand (time(NULL));
    int random = rand();
    Record record(sizeof(int), reinterpret_cast<char*>(&random));
    TID insertedTid = seg.insert(record);
    Record readRecord = seg.lookup(insertedTid);
    int readRandom = *(reinterpret_cast<const int*>(readRecord.getData()));
    assert(random == readRandom);
    return 0;
}
