#!/bin/bash

TIPC_DIR=~/Documents/GitHub/tipc
TIPC_PASS_DIR=~/Documents/GitHub/tipc-passes

${TIPC_BASE_DIR}/build/src/tipc -do $1.tip
# llvm-dis $1.tip.bc
opt -enable-new-pm=0 -load ${TIPC_PASS_BASE_DIR}/build/src/pointstopass/ptpass.so -ptpass < $1.tip.bc >/dev/null 2>$1.ptpass

rm $1.tip.bc
rm $1.tip.ll
