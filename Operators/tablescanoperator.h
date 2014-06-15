#ifndef TableScanOperator_H
#define TableScanOperator_H

#include "operator.h"
#include "register.h"
#include "spsegment.h"

#include <ostream>

class TableScanOperator : public Operator {
    public:

        TableScanOperator(SPSegment& spseg);
        /**
         * @brief ~TableScanOperator
         * Destructor for object of type TableScanOperator
         */
        virtual ~TableScanOperator();

        /**
         * @brief open
         * Open the TableScanOperator
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
         * Close the TableScanOperator
         */
        virtual void close();

    private:
        SPSegment& spsegment;
        unique_ptr<SPRecord_iterator> recordIterator;
        unique_ptr<SPRecord_iterator> endRecordIterator;
};

#endif // TableScanOperator_H
