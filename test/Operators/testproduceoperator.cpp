#include "testproduceoperator.h"

TestProduceOperator::TestProduceOperator()
    : tuplePos(-1) {
    std::vector<Register*> t1;
    auto r = new Register();
    r->setInteger(1);
    t1.push_back(r);
    r = new Register();
    r->setString("Hello World");
    t1.push_back(r);
    r = new Register();
    r->setDouble(10);
    t1.push_back(r);
    tuples.push_back(t1);
    std::vector<Register*> t2;
    r = new Register();
    r->setInteger(2);
    t2.push_back(r);
    r = new Register();
    r->setString("Answer");
    t2.push_back(r);
    r = new Register();
    r->setDouble(42);
    t2.push_back(r);
    tuples.push_back(t2);
    std::vector<Register*> t3;
    r = new Register();
    r->setInteger(3);
    t3.push_back(r);
    r = new Register();
    r->setString("Pi");
    t3.push_back(r);
    r = new Register();
    r->setDouble(3.14159265359);
    t3.push_back(r);
    tuples.push_back(t3);
}


TestProduceOperator::~TestProduceOperator() {
    for (auto it = tuples.begin(); it != tuples.end(); it++) {
        for (auto it2 = it->begin(); it2 != it->end(); it2++) {
            delete *it2;
        }
    }
}

void TestProduceOperator::open() {}

bool TestProduceOperator::next() {
    if (tuplePos >= static_cast<int64_t>(tuples.size()))
        return false;

    return ++tuplePos < static_cast<int64_t>(tuples.size());
}

vector<Register*> TestProduceOperator::getOutput() const {
    if (tuplePos < 0 || tuplePos >= static_cast<int64_t>(tuples.size()))
        throw 0;

    return tuples[tuplePos];
}

void TestProduceOperator::close() {}
