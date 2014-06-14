#include "projectionoperator.h"

ProjectionOperator::ProjectionOperator(Operator& child, vector<uint64_t> projectionIds)
    : child(child),
      projectionIds(projectionIds) {}

ProjectionOperator::~ProjectionOperator() {}

void ProjectionOperator::open() {
    child.open();
}

bool ProjectionOperator::next() {
    auto result = child.next();
    if (result) {
        auto childOutput = child.getOutput();
        currentTuple.clear();
        for (auto it = projectionIds.begin(); it != projectionIds.end(); it++) {
            currentTuple.push_back(childOutput[*it]);
        }
    }

    return result;
}

vector<Register*> ProjectionOperator::getOutput() const {
    return currentTuple;
}

void ProjectionOperator::close() {
    child.close();
}
