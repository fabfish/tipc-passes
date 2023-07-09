#pragma once

#include "PointsTo.h"

#include "llvm/IR/Value.h"

using namespace llvm;

/**
 * @brief LLVM IR中变量的抽象
 * 
 */
class Variable
{
public:
  enum VarClass
  {
    VALUE,
    OBJECT,
  };

protected:
  VarClass kind;
  Value *v;
  PointsTo *ptset;

public:
  Variable(Value *v, VarClass c) : v(v), kind(c)
  {
    ptset = new PointsTo();
  }

  ~Variable()
  {
    delete ptset;
  }

  inline VarClass getVarClass() const { return kind; }

  inline PointsTo *getPointsToSet() { return ptset; }

  virtual std::string str()
  {
    return std::string(v->getName().data());
  }

  std::string pointsToStr();
};

/**
 * @brief 用来表示分配点，即alloc-i。
 * 例如`%d2 = alloca i64, align 8`, `%allocPtr = call i8* @calloc(i64 1, i64 8)`
 *
 */
class Alloc_I : public Variable
{
public:
  Alloc_I(Value *v) : Variable(v, OBJECT) {}

  inline std::string str() override
  {
    return "[" + std::string(v->getName().data()) + "]";
  }

  static bool classof(const Variable *v)
  {
    return v->getVarClass() == OBJECT;
  }
};

/**
 * @brief LLVM IR中的变量，所有用%和@修饰的变量，包括指令、参数、全局变量等
 *
 */
class Value_I : public Variable
{
public:
  Value_I(Value *v) : Variable(v, VALUE) {}

  static bool classof(const Variable *v)
  {
    return v->getVarClass() == VALUE;
  }
};
