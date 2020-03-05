#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct PrintInstPass : public FunctionPass {
        public:
            static char ID;
            PrintInstPass() : FunctionPass(ID) {}
            virtual bool runOnFunction(Function &F) override;
            bool isSupported(Instruction &I);
    };
}

bool PrintInstPass::runOnFunction(Function &F) {
    errs() << "Supported instructions in function: " << F.getName() << "\n";
    for (auto& bb : F) {
      for (auto& i : bb) {
        if (isSupported(i)) {
           errs() << "-->" << i << "\n";
        } else {
           errs() << "   " << i << " {type = " << i.getValueID() << "}\n";
        }
      }
    }
    return false; 
}

bool
PrintInstPass::isSupported(Instruction &i) {
  return (isa<PHINode>(i) ||
          isa<BinaryOperator>(i) ||
          isa<AllocaInst>(i) ||
          isa<LoadInst>(i) ||
          isa<CallInst>(i) ||
          isa<SelectInst>(i) ||
          isa<ICmpInst>(i));
}


// Register the pass with llvm, so that we can call it with pipass
char PrintInstPass::ID = 1;
static RegisterPass<PrintInstPass> X("pipass", "LLVM pass printing each supported TIP instruction");
