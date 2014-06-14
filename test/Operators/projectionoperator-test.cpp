
#include "gtest.h"

#include "printoperator.h"
#include "projectionoperator.h"
#include "testproduceoperator.h"

#include <iostream>

using namespace std;

TEST(Operators, ProjectionOperatorOK) {
    TestProduceOperator sourceOp;
    auto projectionList = vector<uint64_t>({1,2});
    ProjectionOperator projOp(sourceOp, projectionList);
    PrintOperator printOp(projOp, cout);

    cout << "Printing output..." << endl;

    while (printOp.next()) {
        cout << "Printed tuple" << endl;
    }
}

