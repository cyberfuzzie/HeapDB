#include "tablescanoperator.h"
#include "register.h"

TableScanOperator::TableScanOperator(SPSegment& spseg)
    : spsegment(spseg) {}

TableScanOperator::~TableScanOperator() {

}

void TableScanOperator::open() {
    recordIterator = spsegment.getRecordIterator();
    endRecordIterator = recordIterator->end();
}

bool TableScanOperator::next() {
    ++(*recordIterator);
    return (*recordIterator ) == (*endRecordIterator);
}

vector<Register*> TableScanOperator::getOutput() const {
    //TODO: convert record to register array
    return vector<Register*>();
}

void TableScanOperator::close() {
    //destruct iterator
    recordIterator.reset();
}
