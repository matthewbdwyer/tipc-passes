#include <list>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "IntervalRangeAnalysis.h"
#include "Interval.h"

using namespace llvm;

using namespace rangeanalysis;

using namespace interval;

static cl::opt<bool>
    IntervalRangeDebug("intervalrange-debug", cl::Hidden,
                       cl::desc("enable debug output for interval range."));

/* Only handles a limited fragment of the LLVM instructions 
 *    https://llvm.org/docs/LangRef.html#instruction-reference
 * that arise during compilation of TIP programs.
 * Does not handle floats/exceptions/bitwise/poison/undef etc.
 *
 * Could enrich this to filter out unsupported binary op codes.
 */
bool
isSupported(Instruction &i) {
  return (isa<PHINode>(i) ||
          isa<BinaryOperator>(i) ||
          isa<AllocaInst>(i) ||
          isa<LoadInst>(i) ||
          isa<CallInst>(i) ||
          isa<SelectInst>(i) ||
          isa<ICmpInst>(i));
}

// Analysis state (a map lattice)
typedef DenseMap<Value*, Interval> StateMap;

/* Produce an interval value for a value
 *   For constants generate an interval 
 *   For instructions lookup the value in the state
 *
 * This code assumes that the state has been initialized for all
 * instructions.
 */
Interval
getInterval(Value *V, StateMap state) {
  Interval result;
  if (ConstantInt* ci = dyn_cast<ConstantInt>(V)) {
    result = interval::make(ci->getSExtValue(), ci->getSExtValue()); 
  } else {
    result = state.lookup(V);
  }
  return result;
}

// For an analysis pass, runOnFunction should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
IntervalRangeAnalysis::runOnFunction(Function& F) {
  StateMap state;
  std::list<llvm::Instruction*> w; 

  // Initialize the state and worklist for supported instructions
  for (BasicBlock& bb : F) {
    for (Instruction& i : bb) {
      if (isSupported(i)) {
        state[&i] = interval::empty();
        w.push_back(&i);
      }
    }
  }

  if (IntervalRangeDebug) {
    errs() << "DEBUG: initial interval range state for function " << F << "\n";
    for (auto& pair : state) {
      std::string is = str(pair.second);
      errs() << "-->" << *pair.first << " = " << is << "\n";
    }
    errs() << "DEBUG: initial worklist\n";
    for (Instruction* i : w) {
      errs() << "-->" << *i << "\n";
    }
  }

  // Iterate until the worklist is empty
  while (!w.empty()) {
    // Remove the current instruction
    Instruction* i = w.front(); 
    w.pop_front();

    // Record prior value to control worklist insertion
    auto old = state.lookup(i);
    Interval current = interval::empty();

    // Special case handling for each supported instruction type
    if (auto* phi = dyn_cast<PHINode>(i)) {
      // merge all incoming values 
      if (IntervalRangeDebug) {
        errs() << "DEBUG: merging values at node " << phi << "\n";
      }

      for (int idx=0; idx < phi->getNumIncomingValues(); idx++) {
        Interval newlub = lub(current, 
                              getInterval(phi->getIncomingValue(idx), state));

        if (IntervalRangeDebug) {
          errs() << "--> phi[" << idx << "] with lub(" << str(current) << ", ";
          errs() << str(getInterval(phi->getIncomingValue(idx), state));
          errs() << ") = " << str(newlub) << "\n"; 
        }

        current = newlub;
      }

    } else if (auto* si = dyn_cast<SelectInst>(i)) {
      // merge the operands value of the two select cases 
      current = lub(getInterval(si->getTrueValue(), state), 
                    getInterval(si->getFalseValue(), state));

    } else if (auto* bo = dyn_cast<BinaryOperator>(i)) {
      llvm::Instruction::BinaryOps op = bo->getOpcode();
      Interval l = getInterval(bo->getOperand(0), state);
      Interval r = getInterval(bo->getOperand(1), state);

      // Use interval arithmetic semantics 
      if (op == Instruction::Add) {
        current = add(l,r);
      } else if (op == Instruction::Sub) {
        current = sub(l,r);
      } else if (op == Instruction::Mul) {
        current = mul(l,r);
      } else if (op == Instruction::SDiv) {
        current = div(l,r);
      } else {
        // unsupported instruction type, check for this and sharpen filter
        llvm_unreachable("Unsupported BinaryOperator");
      }

    } else if (auto* ic = dyn_cast<ICmpInst>(i)) {
      llvm::CmpInst::Predicate pred = ic->getSignedPredicate();
      Interval l = getInterval(ic->getOperand(0), state);
      Interval r = getInterval(ic->getOperand(1), state);

      // Use comparison expression semantics 
      if (pred == llvm::CmpInst::ICMP_EQ) {
        current = eq(l,r);
      } else if (pred == llvm::CmpInst::ICMP_NE) {
        current = ne(l,r);
      } else if (pred == llvm::CmpInst::ICMP_SLT) {
        current = lt(l,r);
      } else if (pred == llvm::CmpInst::ICMP_SGT) {
        current = gt(l,r);
      } else {
        // unsupported instruction type, check for this and sharpen filter
        llvm_unreachable("Unsupported ICmpInst predicate");
      }

    } else if (isa<AllocaInst>(i) || isa<LoadInst>(i) || isa<CallInst>(i) ) {
      // This is an intra-procedural analysis that does not track memory 
      // locations so these instructions yield a full interval.
      current = interval::full();

    } else {
      llvm_unreachable("Unsupported instruction type");
    }

    if (IntervalRangeDebug) {
      errs() << "DEBUG: analyzing " << *i << "\n";
      errs() << "--> old value = " << str(old) << "\n";
      errs() << "--> new value  =" << str(current) << "\n";
    }

    // add users of this instruction to worklist only if the value has changed
    if (old != current) {
      state[i] = current;
      for(User *u : i->users()) { 
        if (Instruction* cu = dyn_cast<Instruction>(u)){
          if (isSupported(*cu)) {
            // If not scheduled for analysis, add this user
            auto it = std::find(w.begin(), w.end(), cu);
            if (it == w.end()) {
              w.push_back(cu);
              if (IntervalRangeDebug) {
                errs() << "DEBUG: adding to worklist :" << *cu << "\n";
              }
            }
          }
        }
      } 
    }     
  }

  /* Emit the analysis information for the function 
   *   A more useful implementation would record it and make it useful
   *   to other analyses.
   */
  errs() << "*** interval range analysis for function " << F << " ***\n";
  for (auto& pair : state) {
    std::string is = str(pair.second);
    errs() << *pair.first << " = " << is << "\n";
  }

  return false;
}

char IntervalRangeAnalysis::ID = 0;
RegisterPass<IntervalRangeAnalysis> X("irpass",
                                      "Print the interval ranges of locals");
