#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct InstCountPass : public FunctionPass {
        public:
            static char ID;
            InstCountPass() : FunctionPass(ID) {}
            virtual bool runOnFunction(Function &F) override;
    };
}

bool InstCountPass::runOnFunction(Function &F) {
    errs() << "Function " << F.getName() << " contains ";
    int instcount = 0;
    for (auto& bb : F) {
      for (auto& i : bb) {
        instcount++;
      }
    }
    errs() << instcount << " LLVM instructions\n";
    return false; 
}



// Register the pass with llvm, so that we can call it with icpass
char InstCountPass::ID = 1;
static RegisterPass<InstCountPass> X("icpass", "LLVM pass printing number of instructions per function");
