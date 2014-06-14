#ifndef OPERATOR_H
#define OPERATOR_H

#include "register.h"

#include <vector>

using namespace std;

class Operator {
    public:
        /**
         * @brief ~Operator
         * Destructor for object of type Operator
         */
        virtual ~Operator();

        /**
         * @brief open
         * Open the Operator
         */
        virtual void open() = 0;

        /**
         * @brief next
         * Advance to the next tuple
         *
         * @return Returns whether the tuple exists
         */
        virtual bool next() = 0;

        /**
         * @brief getOutput
         * Get the contents of the current tuple
         *
         * @return A vector containing all the field for the tuple
         */
        virtual vector<Register*> getOutput() const = 0;

        /**
         * @brief close
         * Close the Operator
         */
        virtual void close() = 0;
};

#endif // OPERATOR_H
