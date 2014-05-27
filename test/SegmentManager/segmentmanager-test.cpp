
#include "buffermanager.h"
#include "schemamanager.h"
#include "segmentmanager.h"
#include "spsegment.h"
#include "record.h"

#include "gtest.h"

#define PAGESIZE 2048

TEST(SegmentManager, SimpleWriteRead) {
    BufferManager bm(PAGESIZE, 100);
    SchemaManager scm;
    SegmentManager sm(bm, scm);
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
    ASSERT_TRUE(random == readRandom);
}
