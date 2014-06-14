
#include "gtest.h"

#include "printoperator.h"
#include "testoperator.h"

#include <iostream>

using namespace std;

TEST(Operators, PrintOperatorOK) {
    TestOperator testOp;
    PrintOperator printOp(testOp, cout);

    cout << "Printing output..." << endl;

    while (printOp.next()) {
        cout << "Printed tuple" << endl;
    }
}
