#ifndef SelectionOperator_H
#define SelectionOperator_H

#include "operator.h"
#include "register.h"

#include <ostream>

class SelectionOperator : public Operator {
    public:
        /**
         * @brief SelectionOperator Compares each tuple with the constant
         *  in register c at position regID
         * @param child Child operator
         * @param regID Register position for comparison
         * @param c Register holding the constant to compare to
         */
        SelectionOperator(Operator& child, uint32_t regID, Register &c);

        /**
         * @brief ~PrintOperator
         * Destructor for object of type PrintOperator
         */
        virtual ~SelectionOperator();

        /**
         * @brief open
         * Open the SelectionOperator
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
         * @return A vector containing all the fields for the tuple
         */
        virtual vector<Register*> getOutput() const;

        /**
         * @brief close
         * Close the SelectionOperator
         */
        virtual void close();

    private:
        Operator& child;
        uint32_t registerID;
        Register &constant;
        vector<Register*> currentTuple;
};

#endif // PRINTOPERATOR_H
