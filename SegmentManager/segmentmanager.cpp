#include "segmentmanager.h"

#include <fstream>

using std::fstream;

SegmentManager::SegmentManager(BufferManager& bufman)
    : bm(bufman) {
    fstream input("segment0", ios::in | ios::binary);
    bool parseOk = schema.ParseFromIstream(&input);
    input.close();
    if (!parseOk) {
        schema.Clear();
        writeSchema();
    }
}

bool SegmentManager::createSegment(const char* relationName) {
    int32_t nextSegmentId = 0;
    for (int64_t i = 0; i < schema.relations_size(); i++) {
        nextSegmentId = max(nextSegmentId, schema.relations(i).segment_id());
    }
    nextSegmentId++;
    schema::Relation* r = schema.add_relations();
    r->set_name(relationName);
    r->set_segment_id(nextSegmentId);
    r->set_sizeinpages(0);
    writeSchema();
    return true;
}

SPSegment SegmentManager::getSegment(const char* relationName) {
    for (int64_t i = 0; i < schema.relations_size(); i++) {
        if ( schema.relations(i).name().compare(relationName) == 0 ) {
            uint64_t segmentId = schema.relations(i).segment_id();
            uint64_t pageCount = schema.relations(i).sizeinpages();
            return SPSegment(*this, bm, segmentId, pageCount);
        }
    }
    // nothing found
    // TODO: create real exception
    throw 0;
}

void SegmentManager::segmentResized(const SPSegment& segment) {
    for (int64_t i = 0; i < schema.relations_size(); i++) {
        if (schema.relations(i).segment_id() == segment.getSegmentId()) {
            schema.mutable_relations(i)->set_sizeinpages(segment.getPageCount());
        }
    }
    writeSchema();
}

void SegmentManager::writeSchema() {
    fstream output("segment0", ios::out | ios::binary);
    schema.SerializeToOstream(&output);
    output.close();
}
