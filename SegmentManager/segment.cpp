#include "segment.h"
#include "schemamanager.h"

Segment::Segment(SchemaManager& schemaManager, uint64_t segId, uint64_t pgcount, uint32_t ps)
    : scm(schemaManager),
      segmentId{segId},
      pageCount{pgcount},
      pageSize{ps}
{

}

Segment::~Segment(){

}

uint64_t Segment::getSegmentId() const {
    return segmentId;
}

uint64_t Segment::getPageCount() const {
    return pageCount;
}

void Segment::incrementPageCount() {
    pageCount++;
    scm.segmentResized(*this);
}
