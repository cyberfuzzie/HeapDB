#ifndef TESTOPERATOR_H
#define TESTOPERATOR_H

#include "operator.h"

#include <vector>

class TestOperator : public Operator {
    public:
        TestOperator();
        /**
         * @brief ~TestOperator
         * Destructor for object of type TestOperator
         */
        virtual ~TestOperator();

        /**
         * @brief open
         * Open the Operator
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
         * Close the TestOperator
         */
        virtual void close();

    private:
        int tuplePos;
        std::vector<std::vector<Register*>> tuples;
};

#endif // TESTOPERATOR_H
