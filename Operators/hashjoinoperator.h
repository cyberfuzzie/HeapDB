#ifndef HASHJOINOPERATOR_H
#define HASHJOINOPERATOR_H

#include "operator.h"
#include "register.h"

#include <vector>
#include <unordered_map>

class HashJoinOperator : public Operator {
    public:
        /**
         * @brief HashJoinOperator
         * Constructor for HashJoinOperator
         */
        HashJoinOperator(Operator& leftChild, Operator& rightChild, uint64_t leftId, uint64_t rightId);

        /**
         * @brief ~HashJoinOperator
         * Destructor for object of type HashJoinOperator
         */
        virtual ~HashJoinOperator();

        /**
         * @brief open
         * Open the HashJoinOperator
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
         * Close the HashJoinOperator
         */
        virtual void close();

    private:
        struct RegisterHash {
            std::size_t operator()(const Register* reg) const;
        };
        struct RegisterEqual {
            std::size_t operator()(const Register* lhs, const Register* rhs) const;
        };

        Operator& leftChild;
        Operator& rightChild;
        uint64_t leftId;
        uint64_t rightId;
        bool hashMapFilled;
        std::unordered_multimap<Register*,std::vector<Register*>,RegisterHash,RegisterEqual> hashMap;
        bool rightTupleOk;
        std::vector<Register*> rightTuple;
        std::pair<std::unordered_multimap<Register*,
                                          std::vector<Register*>,
                                          RegisterHash,
                                          RegisterEqual>::iterator,
                  std::unordered_multimap<Register*,
                                          std::vector<Register*>,
                                          RegisterHash,
                                          RegisterEqual>::iterator> rightTupleMatches;
        std::vector<Register*> currentTuple;

        bool advanceRight();
        vector<Register*> cloneTuple(const vector<Register*>& source) const;
        void makeHashMap();
};

#endif // HASHJOINOPERATOR_H
