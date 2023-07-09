#!/bin/bash

TIPC_DIR=~/Documents/GitHub/tipc
TIPC_PASS_DIR=~/Documents/GitHub/tipc-passes

# 使用一个 for 循环，将 ptr1 到 7 作为循环变量
for i in {1..7}
do
  # 将循环变量 i 拼接到 ptr 前面，作为传入的参数
  ${TIPC_DIR}/build/src/tipc -do ptr$i.tip
  opt -enable-new-pm=0 -load ${TIPC_PASS_DIR}/build/src/pointstopass/ptpass.so -ptpass < ptr$i.tip.bc >/dev/null 2>ptr$i.ptpass

  rm ptr$i.tip.bc
done