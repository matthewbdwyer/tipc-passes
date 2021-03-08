~/tipc/build/src/tipc -do $1.tip
llvm-dis $1.tip.bc
opt -load ~/tipc-passes/build/src/intervalrangepass/irpass.dylib -mem2reg -irpass < $1.tip.bc >/dev/null 2>$1.irpass
