#include "Andersen.h"

#include <list>

bool isAllocI(Instruction *I)
{
  if (isa<AllocaInst>(I))
    return true;
  else if (CallInst *CI = dyn_cast<CallInst>(I))
  {
    if (!CI->getCalledFunction())
      return false;
    if (CI->getCalledFunction()->getName().equals("calloc"))
      return true;
  }
  return false;
}

/**
 * @brief AllocaInst
 *
 */
bool PTVisitor::visitAllocaInst(AllocaInst &AI)
{
  return false;
}

/**
 * @brief TODO: CastInst
 * 注：所有的CastInst包括：
 *   1. 数值类型int和float之间的转换; FPToSIInst, SIToFPInst等
 *   2. 高位和低位之间的转换; SExtInst, ZExtInst, TruncInst等
 *   3. AddressSpaceCastInst:
 * 将一个指针从一个地址空间转换到另一个地址空间，但不改变指针指向的对象，
 *   多用于多地址空间架构如CPU和GPU之间的异构计算
 *   4. 类型转换; BitCast, PtrToInt, IntToPtr
 */
bool PTVisitor::visitCastInst(CastInst &CI)
{
  return false;
}

/**
 * @brief TODO: LoadInst
 *
 */
bool PTVisitor::visitLoadInst(LoadInst &LI)
{
  return false;
}

/**
 * @brief TODO: StoreInst
 *
 */
bool PTVisitor::visitStoreInst(StoreInst &SI)
{
  return false;
}

/**
 * @brief TODO: PhiNode
 *
 */
bool PTVisitor::visitPHINode(PHINode &phi)
{
  return false;
}

/**
 * @brief TODO: SelectInst
 *
 */
bool PTVisitor::visitSelectInst(SelectInst &SI)
{
  return false;
}

/**
 * @brief TODO: CallInst
 * 注：不要求实现过程间分析。
 *
 */
bool PTVisitor::visitCallInst(CallInst &CI)
{
  return false;
}

/**
 * @brief 不支持指令的默认处理
 *
 */
bool PTVisitor::visitInstruction(Instruction &I)
{
  return false;
}

bool AndersenPointsToAnalysis::runOnFunction(Function &F)
{
  std::list<Instruction *> worklist;

  // Initialize the state for parameters
  for (Argument &arg : F.args())
  {
    Value_I *var = new Value_I(&arg);
    varMap[&arg] = var;
  }

  // Initialize the state and worklist for supported instructions
  for (BasicBlock &BB : F)
  {
    for (Instruction &I : BB)
    {
      worklist.push_back(&I);
      if (isAllocI(&I))
      {
        Alloc_I *obj = new Alloc_I(&I);
        objMap[&I] = obj;
      }
      if (!I.getType()->isVoidTy())
      {
        Value_I *var = new Value_I(&I);
        varMap[&I] = var;
      }
    }
  }

  // Iterate until the worklist is empty
  while (!worklist.empty())
  {
    Instruction *I = worklist.front();
    worklist.pop_front();

    bool changed = ptv.visit(*I);

    if (changed)
    {
      for (User *u : I->users())
      {
        if (Instruction *cu = dyn_cast<Instruction>(u))
        {
          auto it = std::find(worklist.begin(), worklist.end(), cu);
          if (it == worklist.end())
            worklist.push_back(cu);
        }
      }
    }
  }

  /* Emit the analysis information for the function
   *   A more useful implementation would record it and make it useful
   *   to other analyses.
   */
  errs() << "*** Andersen points-to analysis for function " << F << " ***\n";
  for (auto &pair : varMap)
  {
    std::string is = pair.second->pointsToStr();
    errs() << *pair.first << " = " << is << "\n";
  }
  for (auto &pair : objMap)
  {
    std::string is = pair.second->pointsToStr();
    errs() << "[" << *pair.first << "] = " << is << "\n";
  }

  return false;
}

char AndersenPointsToAnalysis::ID = 0;
RegisterPass<AndersenPointsToAnalysis> X("ptpass",
                                         "Print the points-to set of locals");
