#ifndef SCHEMAMANAGER_H
#define SCHEMAMANAGER_H

class SchemaManager;

#include "segment.h"
#include "schema.pb.h"



class SchemaManager
{
public:
    SchemaManager();
    void segmentResized(const Segment& segment);
    schema::Schema& getSchema();
    void writeSchema();
private:
    schema::Schema schema;
};

#endif // SCHEMAMANAGER_H
