#include "hashjoinoperator.h"

HashJoinOperator::HashJoinOperator(Operator &leftChild,
                                   Operator &rightChild,
                                   uint64_t leftId,
                                   uint64_t rightId)
    : leftChild(leftChild),
      rightChild(rightChild),
      leftId(leftId),
      rightId(rightId),
      hashMapFilled(false),
      hashMap(),
      rightTupleOk(false) {}

HashJoinOperator::~HashJoinOperator() {}

void HashJoinOperator::open() {
    rightChild.open();
    hashMapFilled = false;
    hashMap.clear();
}

bool HashJoinOperator::next() {
    if (!hashMapFilled) {
        makeHashMap();
    }

    if (!rightTupleOk) {
        if (!advanceRight())
            return false;
    } else {
        (rightTupleMatches.first)++;
    }

    while (rightTupleMatches.first == rightTupleMatches.second) {
        if (!advanceRight())
            return false;
    }

    // now produce
    currentTuple.clear();
    auto leftTuple = (rightTupleMatches.first)->second;
    for (auto it = leftTuple.begin(); it != leftTuple.end(); it++) {
        currentTuple.push_back(*it);
    }
    for (auto it = rightTuple.begin(); it != rightTuple.end(); it++) {
        currentTuple.push_back(*it);
    }

    return true;
}

vector<Register *> HashJoinOperator::getOutput() const {
    if (!rightTupleOk)
        throw 0;

    return currentTuple;
}

void HashJoinOperator::close() {
    rightChild.close();
}

bool HashJoinOperator::advanceRight() {
    rightTupleOk = rightChild.next();
    if (rightTupleOk) {
        rightTuple = rightChild.getOutput();
        rightTupleMatches = hashMap.equal_range(rightTuple[rightId]);
        return true;
    } else {
        return false;
    }
}

vector<Register*> HashJoinOperator::cloneTuple(const vector<Register*> &source) const {
    vector<Register*> result;
    for (auto it = source.cbegin(); it != source.cend(); it++) {
        result.push_back(new Register(*it));
    }
    return result;
}

void HashJoinOperator::makeHashMap() {
    leftChild.open();
    while (leftChild.next()) {
        auto leftOutput = leftChild.getOutput();
        auto leftTuple = cloneTuple(leftOutput);
        auto leftKey = leftTuple[leftId];
        std::pair<Register*,vector<Register*>> leftPair (leftKey, std::move(leftTuple));
        hashMap.insert(leftPair);
    }
    leftChild.close();
    hashMapFilled = true;
}


size_t HashJoinOperator::RegisterHash::operator()(const Register *reg) const {
    return reg->getHash();
}


size_t HashJoinOperator::RegisterEqual::operator()(const Register *lhs, const Register *rhs) const {
    return *lhs == *rhs;
}
