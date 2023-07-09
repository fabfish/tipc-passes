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
  Value *src = CI.getOperand(0);
  Value *dst = &CI;

  // 如果是指针类型的转换
  if (src->getType()->isPointerTy() && dst->getType()->isPointerTy())
  {
    // 获取转换前后的指针
    Pointer *srcPtr = getOrCreatePointer(src);
    Pointer *dstPtr = getOrCreatePointer(dst);

    // 将它们加入到相同的指向集合中
    unionPointsToSet(srcPtr, dstPtr);
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
  Value *addr = LI.getPointerOperand();
  Value *val = &LI;

  // 如果加载值是指针类型
  if (val->getType()->isPointerTy())
  {
    // 获取加载地址和加载值的指针
    Pointer *addrPtr = getOrCreatePointer(addr);
    Pointer *valPtr = getOrCreatePointer(val);

    // 将加载地址的指针的指向集合赋值给加载值的指针
    copyPointsToSet(addrPtr, valPtr);
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
  Value *addr = SI.getPointerOperand();
  Value *val = SI.getValueOperand();

  // 如果存储值是指针类型
  if (val->getType()->isPointerTy())
  {
    // 获取存储地址和存储值的指针
    Pointer *addrPtr = getOrCreatePointer(addr);
    Pointer *valPtr = getOrCreatePointer(val);

    // 将存储值的指针的指向集合加入到存储地址的指针的指向集合中
    unionPointsToSet(addrPtr, valPtr);
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
  Value *val = &phi;
  Type *ty = phi.getType();

  // 如果PhiNode自身是指针类型
  if (ty->isPointerTy())
  {
    // 获取PhiNode自身的指针
    Pointer *valPtr = getOrCreatePointer(val);

    // 遍历所有可能输入值
    for (unsigned i = 0; i < phi.getNumIncomingValues(); i++)
    {
      // 获取输入值和类型
      Value *inVal = phi.getIncomingValue(i);
      Type *inTy = inVal->getType();
    
      // 如果输入值是指针类型
      if (inTy->isPointerTy())
      {
        // 获取输入值的指针
        Pointer *inPtr = getOrCreatePointer(inVal);
    
        // 将输入值的指针的指向集合加入到PhiNode自身的指针的指向集合中
        unionPointsToSet(valPtr, inPtr);
      }
    }
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
  Value *val = &SI;
  Type *ty = SI.getType();

  // 如果SelectInst自身是指针类型
  if (ty->isPointerTy())
  {
    // 获取SelectInst自身的指针
    Pointer *valPtr = getOrCreatePointer(val);

    // 获取两个可能选择值和类型
    Value *trueVal = SI.getTrueValue();
    Value *falseVal = SI.getFalseValue();
    Type *trueTy = trueVal->getType();
    Type *falseTy = falseVal->getType();
    
    // 如果真值是指针类型
    if (trueTy->isPointerTy())
    {
      // 获取真值的指针
      Pointer *truePtr = getOrCreatePointer(trueVal);
    
      // 将真值的指针的指向集合加入到SelectInst自身的指针的指向集合中
      unionPointsToSet(valPtr, truePtr);
    }
    
    // 如果假值是指针类型
    if (falseTy->isPointerTy())
    {
      // 获取假值的指针
      Pointer *falsePtr = getOrCreatePointer(falseVal);
    
      // 将假值的指针的指向集合加入到SelectInst自身的指针的指向集合中
      unionPointsToSet(valPtr, falsePtr);
    }
  }

  return false;
}

/**
 * @brief CallInst
 * 对于CallInst，我们不需要实现过程间分析，只需要处理一些特殊函数，例如malloc, free等。
 */
bool PTVisitor::visitCallInst(CallInst &CI)
{
  // 获取调用函数和返回值
  Function *func = CI.getCalledFunction();
  Value *retVal = &CI;

  // 如果调用函数是malloc或calloc
  if (func && (func->getName() == "malloc" || func->getName() == "calloc"))
  {
    // 如果返回值是指针类型
    if (retVal->getType()->isPointerTy())
    {
      // 获取返回值的指针
      Pointer *retPtr = getOrCreatePointer(retVal);

      // 创建一个新对象，并将其加入到返回值的指针的指向集合中
      Object *obj = createObject(retVal);
      addPointsToSet(retPtr, obj);
    }
  }

  // 如果调用函数是free
  if (func && func->getName() == "free")
  {
    // 获取第一个参数，即要释放内存的地址
    Value *arg = CI.getArgOperand(0);

    // 如果参数是指针类型
    if (arg->getType()->isPointerTy())
    {
      // 获取参数的指针
      Pointer *argPtr = getOrCreatePointer(arg);
    
      // 清空参数的指针的指向集合，表示释放了内存空间
      clearPointsToSet(argPtr);
    }
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
