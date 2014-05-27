#include "gtest.h"
#include "bplussegment.h"
#include <cstdint>
#include <fstream>
using namespace std;

#define PAGESIZE 2048

TEST(BPlusTree, OnePageReadWrite) {

    BufferManager bm(PAGESIZE, 100);
    SchemaManager scm;
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
        bm, scm, 88, 0, PAGESIZE, 0);

    for (uint64_t i = 1; i < 10; i += 2){
        testTree.insert(i, i * 2);
    }

    for (uint64_t i = 10; 0 < i; i -= 2){
        testTree.insert(i, i * 2);
    }

    for (uint64_t i = 1; i <= 10; i++){
        ASSERT_EQ(i * 2, testTree.lookup(i));
    }
}

TEST(BPlusTree, MultiPageReadWrite) {

    uint pagesize = sizeof(BPlusHeader) + 9 * sizeof(uint64_t);

    BufferManager bm(pagesize, 100);
    SchemaManager scm;
    //creating tree with space for two elements

    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
    bm, scm, 88, 0, pagesize, 0);

    uint elements = 300;

    for (uint64_t i = 1; i <= elements; i++){
        testTree.insert(i, i * 2 + 100);
    }

    for (uint64_t i = 1; i <= elements; i++){
        assert(i * 2 + 100 == testTree.lookup(i));
    }

}

TEST(BPlusTree, VisualizeOutput) {

    BufferManager bm(PAGESIZE, 100);
    SchemaManager scm;
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
        bm, scm, 88, 0, PAGESIZE, 0);

    for (uint64_t i = 1; i < 100; i += 2){
        testTree.insert(i, i * 2);
    }

    filebuf fb;
    fb.open("VisualizeOutput.dot", std::ios::out);
    ostream outFile(&fb);
    testTree.visualize(outFile);
}

