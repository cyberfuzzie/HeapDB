
#include "gtest.h"

#include "hashjoinoperator.h"
#include "printoperator.h"
#include "testproduceoperator.h"

#include <iostream>

using namespace std;

TEST(Operators, HashJoinOperatorOK) {
    TestProduceOperator sourceOp1;
    TestProduceOperator sourceOp2;
    HashJoinOperator joinOp(sourceOp1, sourceOp2, 0, 0);
    PrintOperator printOp(joinOp, cout);

    cout << "Printing output..." << endl;

    while (printOp.next()) {
        cout << "Printed tuple" << endl;
    }
}
