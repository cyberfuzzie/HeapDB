
#include "gtest.h"

#include "printoperator.h"
#include "testproduceoperator.h"

#include <iostream>

using namespace std;

TEST(Operators, PrintOperatorOK) {
    TestProduceOperator sourceOp;
    PrintOperator printOp(sourceOp, cout);

    cout << "Printing output..." << endl;

    while (printOp.next()) {
        cout << "Printed tuple" << endl;
    }
}
