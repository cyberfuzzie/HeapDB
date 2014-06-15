#include "selectionoperator.h"
#include "register.h"

SelectionOperator::SelectionOperator(Operator &child, uint32_t regID, Register& c)
    : child(child),
      registerID{regID},
      constant(c){}

SelectionOperator::~SelectionOperator() {

}

void SelectionOperator::open() {
    child.open();
}

bool SelectionOperator::next() {
    while (child.next()){
        vector<Register *> contents = child.getOutput();
        if (*contents.at(registerID) == constant){

            currentTuple = child.getOutput();
            return true;
        }
    }

    return false;
}

vector<Register*> SelectionOperator::getOutput() const {
    return currentTuple;
}

void SelectionOperator::close() {
    child.close();
}
