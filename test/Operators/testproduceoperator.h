#ifndef TESTOPERATOR_H
#define TESTOPERATOR_H

#include "operator.h"

#include <vector>

class TestProduceOperator : public Operator {
    public:
        TestProduceOperator();
        /**
         * @brief ~TestProduceOperator
         * Destructor for object of type TestProduceOperator
         */
        virtual ~TestProduceOperator();

        /**
         * @brief open
         * Open the TestProduceOperator
         */
        virtual void open();

        /**
         * @brief next
         * Advance to the next tuple
         *
         * @return Returns whether the tuple exists
         */
        virtual bool next();

        /**
         * @brief getOutput
         * Get the contents of the current tuple
         *
         * @return A vector containing all the field for the tuple
         */
        virtual vector<Register*> getOutput() const;

        /**
         * @brief close
         * Close the TestProduceOperator
         */
        virtual void close();

    private:
        int tuplePos;
        std::vector<std::vector<Register*>> tuples;
};

#endif // TESTOPERATOR_H
