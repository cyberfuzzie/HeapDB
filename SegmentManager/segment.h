#ifndef SEGMENT_H
#define SEGMENT_H

#include <cstdint>

// Forward declarations
class Segment;

#include "schemamanager.h"

typedef uint64_t PageID;

class Segment
{
public:

    Segment(SchemaManager& schemaManager, uint64_t segId, uint64_t pgcount, uint32_t pageSize);
    virtual ~Segment();
    uint64_t getSegmentId() const;
    uint64_t getPageCount() const;
    void incrementPageCount();

protected:
    SchemaManager& scm;
    const uint64_t segmentId;
    uint64_t pageCount;
    uint32_t pageSize;
};

#endif // SEGMENT_H
