#include "calcnode.h"

CalcNode::CalcNode(uint32_t varNumber)
    : varNumber(varNumber),
      leaf(true),
      leftChild(nullptr),
      rightChild(nullptr)
{}

CalcNode::CalcNode(CalcOperator op, const CalcNode* leftChild, const CalcNode* rightChild)
    : op(op),
      leaf(false),
      leftChild(leftChild),
      rightChild(rightChild)
{}

bool CalcNode::isLeaf() const {
    return leaf;
}

uint32_t CalcNode::getVarNumber() const {
    return varNumber;
}

CalcNode::CalcOperator CalcNode::getOperator() const {
    return op;
}

const CalcNode &CalcNode::getLeftChild() const {
    return *leftChild;
}

const CalcNode &CalcNode::getRightChild() const {
    return *rightChild;
}
