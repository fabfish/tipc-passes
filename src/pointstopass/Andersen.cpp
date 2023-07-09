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
  Alloc_I *obj = context->findAllocI(&AI);
  PointsTo *valPT = context->findValueI(&AI)->getPointsToSet();
  return valPT->insert(obj);
}

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
