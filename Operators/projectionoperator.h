#ifndef PROJECTIONOPERATOR_H
#define PROJECTIONOPERATOR_H

#include "operator.h"
#include <vector>

class ProjectionOperator : public Operator {
    public:
        /**
         * @brief ProjectionOperator
         * Constructor for ProjectionOperator
         */
        ProjectionOperator(Operator& child, vector<uint64_t> projectionIds);

        /**
         * @brief ~ProjectionOperator
         * Destructor for object of type ProjectionOperator
         */
        virtual ~ProjectionOperator();

        /**
         * @brief open
         * Open the ProjectionOperator
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
         * Close the ProjectionOperator
         */
        virtual void close();

    private:
        Operator& child;
        vector<uint64_t> projectionIds;
        vector<Register*> currentTuple;
};

#endif // PROJECTIONOPERATOR_H
