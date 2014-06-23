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

int main(int argc, char **argv) {
    CalcNode v0(0);
    CalcNode v1(1);
    CalcNode v2(2);
    CalcNode v3(3);
    CalcNode add(CalcNode::CalcOperator::add, &v0, &v1);
    CalcNode sub(CalcNode::CalcOperator::subtract, &v2, &v3);
    CalcNode mul(CalcNode::CalcOperator::multiply, &add, &sub);

    InitializeNativeTarget();
    LLVMContext Context;

    // Create some module to put our function into it.
    OwningPtr<Module> M(new Module("test", Context));

    Type* ty=Type::getInt32Ty(Context);
    Function* f = CreateCalcFunction(M.get(), Context, ty, mul);

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
    for (int i=1; i<argc; i++) {
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
