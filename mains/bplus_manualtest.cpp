
#include <stdio.h>

#include "bplussegment.h"
#include <cstdint>
#include <iostream>

using namespace std;

#define PAGESIZE 2048

int main()
{

    BufferManager bm(PAGESIZE, 100);
    SchemaManager scm;
    //creating tree with space for two elements
    //int pagesize = sizeof(BPlusHeader) + 9 * sizeof(uint64_t);
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
    bm, scm, 88, 0, PAGESIZE, 0);

    for (uint64_t i = 1; i <= 30; i++){
        testTree.insert(i, i * 2 + 100);
    }

    testTree.visualize(cout);

    for (uint64_t i = 1; i <= 30; i++){
        assert(i * 2 + 100 == testTree.lookup(i));
    }

    return 0;

}
