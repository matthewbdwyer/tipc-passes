#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct FunVisitPass : public FunctionPass {
        public:
            static char ID;
            FunVisitPass() : FunctionPass(ID) {}
            virtual bool runOnFunction(Function &F) override;
    };
}


bool FunVisitPass::runOnFunction(Function &F) {
    errs() << "Visiting function: " << F.getName() << "\n";
    return false; 
}

// Register the pass with llvm, so that we can call it with fvpass
char FunVisitPass::ID = 0;
static RegisterPass<FunVisitPass> X("fvpass", "Example LLVM pass printing each function it visits");
