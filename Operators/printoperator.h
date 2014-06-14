#ifndef PRINTOPERATOR_H
#define PRINTOPERATOR_H

#include "operator.h"
#include "register.h"

#include <ostream>

class PrintOperator : public Operator {
    public:
        PrintOperator(Operator& child, std::ostream& output);
        /**
         * @brief ~PrintOperator
         * Destructor for object of type PrintOperator
         */
        virtual ~PrintOperator();

        /**
         * @brief open
         * Open the PrintOperator
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
         * Close the PrintOperator
         */
        virtual void close();

    private:
        Operator& child;
        std::ostream& output;
};

#endif // PRINTOPERATOR_H
