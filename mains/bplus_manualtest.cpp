
#include <stdio.h>

#include "bplussegment.h"
#include <cstdint>

using namespace std;


int main()
{

    BufferManager bm(100);
    SchemaManager scm;
    //creating tree with space for two elements
    int pagesize = sizeof(BPlusHeader) + 9 * sizeof(uint64_t);
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
    bm, scm, 88, 0, pagesize, 0);

    for (uint64_t i = 1; i < 100; i++){
        testTree.insert(i, i * 2 + 100);
    }

    for (uint64_t i = 1; i < 100; i++){
        assert(i * 2 + 100 == testTree.lookup(i));
    }

    return 1;

}
