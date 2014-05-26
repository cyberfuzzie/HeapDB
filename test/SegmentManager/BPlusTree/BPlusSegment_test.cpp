#include "gtest.h"
#include "bplussegment.h"
#include <cstdint>
#include <fstream>
using namespace std;

TEST(BPlusTree, OnePageReadWrite) {

    BufferManager bm(100);
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

    BufferManager bm(100);
    SchemaManager scm;
    //creating tree with space for two elements
    int pagesize = sizeof(BPlusHeader) + 9 * sizeof(uint64_t);
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
    bm, scm, 88, 0, pagesize, 0);

    for (uint64_t i = 1; i < 100; i++){
        testTree.insert(i, i * 2);
    }

    for (uint64_t i = 1; i < 100; i++){
        ASSERT_EQ(i * 2, testTree.lookup(i));
    }

}

TEST(BPlusTree, VisualizeOutput) {

    BufferManager bm(100);
    SchemaManager scm;
    BPlusSegment<uint64_t, uint64_t> testTree([](const uint64_t& a,const uint64_t& b){return a < b;},
        bm, scm, 88, 0, PAGESIZE, 0);

    for (uint64_t i = 1; i < 1000; i += 2){
        testTree.insert(i, i * 2);
    }

    filebuf fb;
    fb.open("VisualizeOutput.dot", std::ios::out);
    ostream outFile(&fb);
    testTree.visualize(outFile);
}

