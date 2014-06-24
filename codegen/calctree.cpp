#include <cstring>
#include <iostream>
#include <memory>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include "calcnode.h"

using namespace llvm;
using namespace std;

Value* ProcessNode(LLVMContext& Context, Function* func, BasicBlock *block, Type* dataType, Type* argType, const CalcNode& node) {
    Value* result;

    if (node.isLeaf()) {
        Argument* arg = func->arg_begin();
        Value* argOffset = ConstantInt::get(dataType, node.getVarNumber());
        Value* argPos = GetElementPtrInst::Create(arg, argOffset, "argPos", block);
        result = new LoadInst(argPos, "arg", block);
        return result;
    }

    Value* callLeft = ProcessNode(Context, func, block, dataType, argType, node.getLeftChild());
    Value* callRight = ProcessNode(Context, func, block, dataType, argType, node.getRightChild());

    switch (node.getOperator()) {
        case CalcNode::CalcOperator::add:
            result = BinaryOperator::CreateAdd(callLeft, callRight, "result", block);
            break;
        case CalcNode::CalcOperator::subtract:
            result = BinaryOperator::CreateSub(callLeft, callRight, "result", block);
            break;
        case CalcNode::CalcOperator::multiply:
            result = BinaryOperator::CreateMul(callLeft, callRight, "result", block);
            break;
        case CalcNode::CalcOperator::divide:
            result = BinaryOperator::CreateSDiv(callLeft, callRight, "result", block);
            break;
    }

    return result;
}

static Function *CreateCalcFunction(Module *M, LLVMContext &Context, Type* dataType, const CalcNode& root) {
    Type* argType = PointerType::get(dataType, 0);
    Function *CalcF = cast<Function>(M->getOrInsertFunction("calc", dataType, argType, nullptr));

    BasicBlock *block = BasicBlock::Create(Context, "Block", CalcF);

    Value* result = ProcessNode(Context, CalcF, block, dataType, argType, root);

    ReturnInst::Create(Context, result, block);

    return CalcF;
}

CalcNode* createTree(char* input) {
    char* tok = strtok(input, " ");
    stack<CalcNode*> nodeStack;
    CalcNode* rhs;
    CalcNode* lhs;
    while (tok != nullptr) {
        switch (tok[0]) {
            case '+':
                rhs = nodeStack.top();
                nodeStack.pop();
                lhs = nodeStack.top();
                nodeStack.pop();
                nodeStack.push(new CalcNode(CalcNode::CalcOperator::add, lhs, rhs));
                break;
            case '-':
                rhs = nodeStack.top();
                nodeStack.pop();
                lhs = nodeStack.top();
                nodeStack.pop();
                nodeStack.push(new CalcNode(CalcNode::CalcOperator::subtract, lhs, rhs));
                break;
            case '*':
                rhs = nodeStack.top();
                nodeStack.pop();
                lhs = nodeStack.top();
                nodeStack.pop();
                nodeStack.push(new CalcNode(CalcNode::CalcOperator::multiply, lhs, rhs));
                break;
            case '/':
                rhs = nodeStack.top();
                nodeStack.pop();
                lhs = nodeStack.top();
                nodeStack.pop();
                nodeStack.push(new CalcNode(CalcNode::CalcOperator::divide, lhs, rhs));
                break;
            default:
                int var = atoi(tok);
                nodeStack.push(new CalcNode(var));
        }

        tok = strtok(nullptr, " ");
    }
    return nodeStack.top();
}

void printHelp() {
    cout << "Too few arguments" << endl << endl;
    cout << "Usage: calctree <tree string> <variable values...>" << endl << endl;
    cout << "\t<tree string>: the tree in reverse polish notation" << endl;
    cout << "\t               e.g. \"0 1 + 2 3 - *\" gives the following tree:" << endl;
    cout << "\t                  *" << endl;
    cout << "\t                 / \\" << endl;
    cout << "\t                +   -" << endl;
    cout << "\t               / \\ / \\" << endl;
    cout << "\t               0 1 2 3" << endl << endl;
    cout << "\t<variable values...>: values for the variables used in the tree" << endl;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        printHelp();
        return 1;
    }

    std::unique_ptr<CalcNode> tree(createTree(argv[1]));

    InitializeNativeTarget();
    LLVMContext Context;

    // Create some module to put our function into it.
    OwningPtr<Module> M(new Module("test", Context));

    Type* ty=Type::getInt32Ty(Context);
    Function* f = CreateCalcFunction(M.get(), Context, ty, *tree);

    auto engine=EngineKind::JIT;
    std::string errStr;
    ExecutionEngine *EE =
      EngineBuilder(M.get())
      .setErrorStr(&errStr)
      .setEngineKind(engine)
      .create();

    if (!EE) {
      errs() << argv[0] << ": Failed to construct ExecutionEngine: " << errStr
             << "\n";
      return 1;
    }

    errs() << "verifying... ";
    if (verifyModule(*M)) {
      errs() << argv[0] << ": Error constructing function!\n";
      return 1;
    }

    errs() << "OK\n";
    errs() << "We just constructed this LLVM module:\n\n---------\n" << *M;

    std::vector<uint32_t> argList;
    for (int i=2; i<argc; i++) {
        argList.push_back(atoi(argv[i]));
    }

    errs() << "\nCalling calc with arguments ";
    for (auto it = argList.begin(); it != argList.end(); it++) {
        errs() << *it << " ";
    }

    std::vector<GenericValue> Args(1);
    Args[0].PointerVal = argList.data();
    GenericValue GV = EE->runFunction(f, Args);

    // import result of execution
    outs() << "\nResult: " << GV.IntVal << "\n";

    return 0;
}
