#pragma once

#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/Value.h"

#include <string>

using namespace llvm;

class Alloc_I;

typedef DenseSet<Alloc_I *> SetType;

class PointsTo
{
private:
  SetType set;

public:
  PointsTo() {}
  ~PointsTo() {}

  inline SetType &getSet() { return set; }

  inline bool insert(Alloc_I *v)
  {
    if (set.count(v))
      return false;
    set.insert(v);
    return true;
  }

  bool unionSets(PointsTo *other)
  {
    bool result = false;
    for (auto &element : other->getSet())
    {
      result |= insert(element);
    }
    return result;
  }

  inline bool empty() { return set.empty(); }
};
