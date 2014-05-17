#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

// Forward declaration
class SegmentManager;

#include "buffermanager.h"
#include "schemamanager.h"
#include "segment.h"
#include "spsegment.h"

#include "schema.pb.h"

class SegmentManager {
    public:
        SegmentManager(BufferManager& bufman, SchemaManager& schemaManager);
        bool createSegment(const char* relationName);
        SPSegment getSegment(const char* relationName);
    private:
        BufferManager& bm;
        SchemaManager& scm;
};

#endif // SEGMENTMANAGER_H
