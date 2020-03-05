~/tipc/build/tipc -d $1.tip
llvm-dis-9 $1.tip.bc
opt-9 -load ~/tipc-passes/build/src/intervalrangepass/irpass.so -mem2reg -irpass < $1.tip.bc >/dev/null 2>$1.irpass
