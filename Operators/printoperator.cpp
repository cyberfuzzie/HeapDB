#include "printoperator.h"
#include "register.h"

PrintOperator::PrintOperator(Operator &child, ostream &output)
    : child(child),
      output(output) {}

PrintOperator::~PrintOperator() {

}

void PrintOperator::open() {
    child.open();
}

bool PrintOperator::next() {
    auto result = child.next();
    if (result) {
        auto contents = child.getOutput();
        auto fieldNr = 0;
        for (auto it = contents.begin(); it != contents.end(); it++) {
            output << "Field " << fieldNr << ": ";
            if ((*it)->getType() == Register::Integer) {
                output << "(Integer) " << (*it)->getInteger() << std::endl;
            } else if ((*it)->getType() == Register::Double) {
                output << "(Double) " << (*it)->getDouble() << std::endl;
            } else if ((*it)->getType() == Register::String) {
                output << "(String) " << (*it)->getString() << std::endl;
            }
            fieldNr++;
        }
        // print it
    }
    return result;
}

vector<Register*> PrintOperator::getOutput() const {
    return vector<Register*>();
}

void PrintOperator::close() {
    child.close();
}
