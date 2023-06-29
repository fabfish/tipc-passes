#pragma once

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "PointsTo.h"
#include "Variable.h"

using namespace llvm;

class AndersenPointsToAnalysis;

/**
 * @brief visitor mode
 *
 */
class PTVisitor : public InstVisitor<PTVisitor, bool>
{
private:
  // context of points-to analysis
  AndersenPointsToAnalysis *context;

public:
  PTVisitor(AndersenPointsToAnalysis *ra) : context(ra) {}

  bool visitAllocaInst(AllocaInst &AI);

  bool visitCastInst(CastInst &CI);

  bool visitLoadInst(LoadInst &LI);

  bool visitStoreInst(StoreInst &SI);

  bool visitPHINode(PHINode &phi);

  bool visitSelectInst(SelectInst &SI);

  bool visitCallInst(CallInst &CI);

  bool visitInstruction(Instruction &I);
};

class AndersenPointsToAnalysis : public FunctionPass
{
  typedef DenseMap<Value *, Value_I *> Value_IMAP;
  typedef DenseMap<Value *, Alloc_I *> Alloc_IMAP;

private:
  Value_IMAP varMap;
  Alloc_IMAP objMap;
  PTVisitor ptv;

public:
  static char ID;

  AndersenPointsToAnalysis() : FunctionPass(ID), ptv(this) {}
  ~AndersenPointsToAnalysis()
  {
    for (auto element : varMap)
    {
      delete element.second;
    }

    for (auto element : objMap)
    {
      delete element.second;
    }
  }

  inline Value_I *findValueI(Value *v) {
    if (varMap.find(v) == varMap.end()) return nullptr;
    else return varMap[v];
  }

  inline Alloc_I *findAllocI(Value *v) {
    if (objMap.find(v) == objMap.end()) return nullptr;
    else return objMap[v];
  }

  bool runOnFunction(Function &f) override;
};
