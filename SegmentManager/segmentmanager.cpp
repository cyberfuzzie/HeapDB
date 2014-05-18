#include "segmentmanager.h"

#include "schemamanager.h"

#include <fstream>

using std::fstream;

SegmentManager::SegmentManager(BufferManager& bufman, SchemaManager &schemaManager)
    : bm(bufman),
      scm(schemaManager)
{

}

bool SegmentManager::createSegment(const char* relationName) {
    uint32_t nextSegmentId = 0;
    for (int64_t i = 0; i < scm.getSchema().relations_size(); i++) {
        nextSegmentId = max(nextSegmentId, scm.getSchema().relations(i).segment().segment_id());
    }
    nextSegmentId++;
    schema::Relation* r = scm.getSchema().add_relations();
    r->set_name(relationName);
    r->mutable_segment()->set_segment_id(nextSegmentId);
    r->mutable_segment()->set_sizeinpages(0);
    scm.writeSchema();
    return true;
}

SPSegment SegmentManager::getSegment(const char* relationName) {
    for (int64_t i = 0; i < scm.getSchema().relations_size(); i++) {
        if ( scm.getSchema().relations(i).name().compare(relationName) == 0 ) {
            uint64_t segmentId = scm.getSchema().relations(i).segment().segment_id();
            uint64_t pageCount = scm.getSchema().relations(i).segment().sizeinpages();
            return SPSegment(scm, bm, segmentId, pageCount, PAGESIZE);
        }
    }
    // nothing found
    // TODO: create real exception
    throw 0;
}


