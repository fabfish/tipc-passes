# 实验报告

SA22011090 余致远

## 选题

基于TIP语言的Andersen指向分析

ref: [tipc-passes/src/pointstopass at master · ustc-pldpa/tipc-passes · GitHub](https://github.com/ustc-pldpa/tipc-passes/tree/master/src/pointstopass#readme)

ref: new bing

为了简化，该指向分析是过程内的、流不敏感、域不敏感的，且仅需求解函数结束时的各个变量的指向集合

## 设计思路

见代码注释：

```cpp
/**
 * @brief CastInst
 * 对于CastInst，我们需要考虑两种情况：
 *   1. 如果是指针类型的转换，例如BitCast, PtrToInt, IntToPtr等，
 *   那么我们需要将转换前后的指针加入到相同的指向集合中，表示它们指向相同的对象。
 *   2. 如果是数值类型的转换，例如FPToSIInst, SIToFPInst等，
 *   那么我们可以忽略这种转换，因为它不影响指针分析的结果。
   */
bool PTVisitor::visitCastInst(CastInst &CI)
{
  // 获取转换前后的值
  Value_I *src = context->findValueI(CI.getOperand(0));
  Value_I *dst = context->findValueI(&CI);

  // 如果是指针类型的转换
  if (!src || !dst) {
    return false;
  }
  else {
    // 获取转换前后的指针
    PointsTo *srcPtr = src->getPointsToSet();
    PointsTo *dstPtr = dst->getPointsToSet();
    return dstPtr->unionSets(srcPtr);
  }
  return false;
}

/**
 * @brief LoadInst
 * 对于LoadInst，我们需要将加载地址的指针的指向集合赋值给加载值的指针，
 * 表示加载值的指针可以指向加载地址的指针所指向的对象。
 */
bool PTVisitor::visitLoadInst(LoadInst &LI)
{
  // 获取加载地址和加载值
  Value_I *src = context->findValueI(LI.getPointerOperand());
  Value_I *dst = context->findValueI(&LI);

  // 如果加载值是指针类型, 获取加载地址和加载值的指针, 将加载地址的指针的指向集合赋值给加载值的指针
  if (!src || !dst) {
    return false;
  }
  else {
    bool result = false;
    PointsTo *srcPtr = src->getPointsToSet();
    PointsTo *dstPtr = dst->getPointsToSet();
    for (auto &element : srcPtr->getSet())
    {
      result |= dstPtr->unionSets(element->getPointsToSet());
    }
    return result;
  }
  return false;
}

/**
 * @brief StoreInst
 * 对于StoreInst，我们需要将存储值的指针的指向集合加入到存储地址的指针的指向集合中，
 * 表示存储地址的指针可以指向存储值的指针所指向的对象。
 */
bool PTVisitor::visitStoreInst(StoreInst &SI)
{
  // 获取存储地址和存储值
  Value_I *src = context->findValueI(SI.getValueOperand());
  Value_I *dst = context->findValueI(SI.getPointerOperand());

  // 如果存储值是指针类型, 获取存储地址和存储值的指针, 将存储值的指针的指向集合加入到存储地址的指针的指向集合中
  if (!src || !dst) {
    return false;
  }
  else {
    bool result = false;
    PointsTo *srcPtr = src->getPointsToSet();
    PointsTo *dstPtr = dst->getPointsToSet();
    for (auto &element : dstPtr->getSet())
    {
      result |= element->getPointsToSet()->unionSets(srcPtr);
    }
    return result;
  }

  return false;
}

/**
 * @brief PhiNode
 * 对于PhiNode，我们需要将所有可能输入值的指针的指向集合加入到PhiNode自身的指针的指向集合中，
 * 表示PhiNode自身的指针可以根据不同分支条件而指向不同输入值所指向的对象。
 */
bool PTVisitor::visitPHINode(PHINode &phi)
{
  // 获取PhiNode自身的值和类型
  Value_I *val = context->findValueI(&phi);

  // 如果PhiNode自身是指针类型
  if (!val) {
    return false;
  } 
  else {
    bool result = false;
    // 获取PhiNode自身的指针
    PointsTo *valPtr = val->getPointsToSet();

    // 遍历所有可能输入值
    for (unsigned i = 0; i < phi.getNumIncomingValues(); i++)
    {
      // 获取输入值和类型
      Value_I *inVal = context->findValueI(phi.getIncomingValue(i));
    
      // 如果输入值是指针类型
      if (inVal)
      {
        // 获取输入值的指针, 将输入值的指针的指向集合加入到PhiNode自身的指针的指向集合中
        result |= valPtr->unionSets(inVal->getPointsToSet());
      }
    }
    return result;
  }

  return false;
}

/**
 * @brief SelectInst
 * 对于SelectInst，我们需要将两个可能选择值的指针的指向集合加入到SelectInst自身的指针的指向集合中，
 * 表示SelectInst自身的指针可以根据不同选择条件而指向不同选择值所指向的对象。
 */
bool PTVisitor::visitSelectInst(SelectInst &SI)
{
  // 获取SelectInst自身的值和类型
  Value_I *val = context->findValueI(&SI);

  // 如果SelectInst自身是指针类型
  if (!val) {
    return false;
  }
  else {
    bool result = false;
    // 获取SelectInst自身的指针
    PointsTo *valPtr = val->getPointsToSet();

    // 获取两个可能选择值和类型
    Value_I *trueVal = context->findValueI(SI.getTrueValue());
    Value_I *falseVal = context->findValueI(SI.getFalseValue());
    
    // 如果真值是指针类型
    if (trueVal)
    {
      // 获取真值的指针
      PointsTo *truePtr = trueVal->getPointsToSet();
    
      // 将真值的指针的指向集合加入到SelectInst自身的指针的指向集合中
      result |= valPtr->unionSets(truePtr);
    }
    
    // 如果假值是指针类型
    if (falseVal)
    {
      // 获取假值的指针
      PointsTo *falsePtr = falseVal->getPointsToSet();
    
      // 将假值的指针的指向集合加入到SelectInst自身的指针的指向集合中
      result |= valPtr->unionSets(falsePtr);
    }
    return result;
  }

  return false;
}

/**
 * @brief CallInst
 * 对于CallInst，我们不需要实现过程间分析，只需要处理一些特殊函数，例如malloc, free等。
 */
bool PTVisitor::visitCallInst(CallInst &CI)
{
  Alloc_I *src = context->findAllocI(&CI);
  Value_I *dst = context->findValueI(&CI);
  if (!src || !dst) {
    return false;
  }
  else {
    PointsTo *dstPtr = dst->getPointsToSet();
    return dstPtr->insert(src);
  }
  return false;
}
```

## 测试方式和结果

使用 run.sh 进行测试，结果见各 ptpass

```shell
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
```



## 问题和建议

问题：主要是 cmake 时 git clone 很慢，本来以为可以在 wsl 内完成，为了方便只好换到有代理的 Ubuntu 机器。

另外有一些 llvm dependency 需要解决，使用了 (for Jammy (22.04)) [CMake error in bcc source compilation · Issue #4462 · iovisor/bcc (github.com)](https://github.com/iovisor/bcc/issues/4462)

> sudo apt install -y zip bison build-essential cmake flex git libedit-dev \
>   libllvm14 llvm-14-dev libclang-14-dev python3 zlib1g-dev libelf-dev libfl-dev python3-setuptools \
>   liblzma-dev libdebuginfod-dev arping netperf iperf

建议提供自动测试和 ptpass 验证脚本。