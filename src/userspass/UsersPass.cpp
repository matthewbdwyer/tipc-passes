#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct UsersPass : public FunctionPass {
        public:
            static char ID;
            UsersPass() : FunctionPass(ID) {}
            virtual bool runOnFunction(Function &F) override;
            bool isSupported(Instruction &I);
    };
}

bool UsersPass::runOnFunction(Function &F) {
    errs() << "Supported uses of supported instructions in function: " << F.getName() << "\n";
    for (BasicBlock& bb : F) {
      for (Instruction& i : bb) {
        if (isSupported(i)) {
          errs() << i << "\n";
          for (User *u : i.users()) { 
            if (Instruction *ui = dyn_cast<Instruction>(u)) {
              if (isSupported(*ui)) {
                errs() << "  " << *ui << "\n";
              }
            }
          } 
        }
      }
    }
    return false; 
}

bool
UsersPass::isSupported(Instruction &i) {
  return (isa<PHINode>(i) ||
          isa<BinaryOperator>(i) ||
          isa<AllocaInst>(i) ||
          isa<LoadInst>(i) ||
          isa<CallInst>(i) ||
          isa<SelectInst>(i) ||
          isa<ICmpInst>(i));
}


// Register the pass with llvm, so that we can call it with pipass
char UsersPass::ID = 0;
static RegisterPass<UsersPass> X("userspass", "LLVM pass printing each supported TIP instruction and its users");
