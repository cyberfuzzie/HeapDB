#ifndef CALCNODE_H
#define CALCNODE_H

#include <cstdint>

class CalcNode {
    public:
        enum CalcOperator {
            add,
            subtract,
            multiply,
            divide
        };

        CalcNode(uint32_t varNumber);
        CalcNode(CalcOperator op, const CalcNode* leftChild, const CalcNode* rightChild);

        bool isLeaf() const;
        uint32_t getVarNumber() const;
        CalcOperator getOperator() const;
        const CalcNode& getLeftChild() const;
        const CalcNode& getRightChild() const;

    private:
        union {
            uint32_t varNumber;
            CalcOperator op;
        };

        bool leaf;

        const CalcNode* leftChild;
        const CalcNode* rightChild;
};

#endif // CALCNODE_H
