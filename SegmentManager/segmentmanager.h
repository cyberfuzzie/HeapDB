#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

typedef int SPSegment;

class SegmentManager
{
    public:
        SegmentManager();
        bool createSegment(const char* relationName);
        SPSegment& getSegment(const char* relationName);
};

#endif // SEGMENTMANAGER_H
