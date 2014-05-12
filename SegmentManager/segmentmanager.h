#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

// Forward declaration
class SegmentManager;

#include "buffermanager.h"
#include "spsegment.h"

#include "schema.pb.h"

class SegmentManager {
    public:
        SegmentManager(BufferManager& bufman);
        bool createSegment(const char* relationName);
        SPSegment getSegment(const char* relationName);
        void segmentResized(const SPSegment& segment);
    private:
        void writeSchema();
        BufferManager& bm;
        schema::Schema schema;
};

#endif // SEGMENTMANAGER_H
