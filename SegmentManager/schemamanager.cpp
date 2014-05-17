#include "schemamanager.h"

#include <fstream>

using namespace std;

SchemaManager::SchemaManager()
{
    fstream input("segment0", ios::in | ios::binary);
    bool parseOk = schema.ParseFromIstream(&input);
    input.close();
    if (!parseOk) {
        schema.Clear();
        writeSchema();
    }
}

void SchemaManager::segmentResized(const Segment& segment) {
    for (int64_t i = 0; i < schema.relations_size(); i++) {
        if (schema.relations(i).segment().segment_id() == segment.getSegmentId()) {
            schema.mutable_relations(i)->mutable_segment()->set_sizeinpages(segment.getPageCount());
        }
    }
    writeSchema();
}

schema::Schema& SchemaManager::getSchema()
{
    return schema;
}

void SchemaManager::writeSchema() {
    fstream output("segment0", ios::out | ios::binary);
    schema.SerializeToOstream(&output);
    output.close();
}
