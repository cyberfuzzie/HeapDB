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
    // TODO: not implemented
    //schema::Relation* r = s.add_relations();
    //r->set_name(relationName);
    //r->set_segment_id();
    //r->set_sizeinpages(0);
    return false;
}

SPSegment SegmentManager::getSegment(const char* relationName) const {
    for (int64_t i = 0; i < schema.relations_size(); i++) {
        if ( schema.relations(i).name().compare(relationName) == 0 ) {
            uint64_t segmentId = schema.relations(i).segment_id();
            uint64_t pageCount = schema.relations(i).sizeinpages();
            return SPSegment(bm, segmentId, pageCount);
        }
    }
    // nothing found
    // TODO: create real exception
    throw 0;
}

void SegmentManager::writeSchema() {
    fstream output("segment0", ios::out | ios::binary);
    schema.SerializeToOstream(&output);
    output.close();
}
