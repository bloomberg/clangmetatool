#include "propagation_visitor.h"
#include "constant_propagator.h"

#include <clangmetatool/propagation/constant_integer_propagator.h>

#include <string>

#include <clang/Analysis/CFG.h>
#include <clang/AST/Expr.h>
#include <llvm/ADT/APSInt.h>

namespace clangmetatool {
namespace propagation {
namespace {

// Given a type, determine if it is a int
bool isIntegerType(const clang::Type* T) {
  return T->isIntegerType();
}

bool isPtrToIntegerType(const clang::Type* T) {
  return T->isPointerType() && isIntegerType(T->getPointeeType().getTypePtr());
}

// Utility class to visit the statements of a block and update the
// ValueContextMap in the process
class IntegerVisitor : public PropagationVisitor<IntegerVisitor, std::string> {
private:
  // Given an expression, try to evaluate it to a int result. Return
  // false if this is not possible
  bool evalExprToInteger(std::string& result, const clang::Expr* E) {
    // We only care about char types
    if(isIntegerType(E->getType().getTypePtr())) {
      clang::Expr::EvalResult ER;

      if(E->isEvaluatable(context) && E->EvaluateAsRValue(ER, context)) {
        // TODO: Where can this fail?
        result = ER.Val.getAsString(context, E->getType());
        return true;
      }
    }

    return false;
  }

  bool evalCompoundExprToInteger(std::string& result, const clang::Expr* E) {
    llvm::APSInt ER;
    E->EvaluateAsInt(ER, context);
    result = ER.toString(10);
    return true;
  }

public:
  // Use parent class's constructor
  using PropagationVisitor<IntegerVisitor, std::string>::PropagationVisitor;

  // Visit a declaration of a variable
  void VisitDeclStmt(const clang::DeclStmt* DS) {
    for(auto D : DS->decls()) {
      if(clang::Decl::Var == D->getKind()) {
        auto VD = reinterpret_cast<const clang::VarDecl*>(D);

        // Only the local non-static variables are possibly deterministic
        if(VD->isLocalVarDecl() && VD->hasLocalStorage()) {
          if(VD->hasInit()) {
            auto I = VD->getInit();

            std::string result;

            if(evalExprToInteger(result, I)) {
              // If the variable is a string, add it to the map
              addToMap(VD->getNameAsString(), result, VD->getLocStart());
            }
          }
        }
      }
    }
  }

  // Visit a binary operator
  void VisitBinaryOperator(const clang::BinaryOperator* BO) {
    // We only care about assignments (=) in this context
    if(clang::BO_Assign == BO->getOpcode()) {
      if(clang::Stmt::DeclRefExprClass == BO->getLHS()->getStmtClass()) {
        auto LHS = reinterpret_cast<const clang::DeclRefExpr*>(BO->getLHS());

        std::string result;

        if(evalExprToInteger(result, BO->getRHS())) {
            // If we can evaluate the expression to a string add the result
            // to the context map
            addToMap(LHS->getNameInfo().getAsString(), result, BO->getLocStart());
        } else {
            // Otherwise, mark the variable as UNRESOLVED after this point
            addToMap(LHS->getNameInfo().getAsString(), {}, BO->getLocStart());
        }
      }
    }
    else if(BO->isCompoundAssignmentOp()) {
      if(clang::Stmt::DeclRefExprClass == BO->getLHS()->getStmtClass()) {
        auto LHS = reinterpret_cast<const clang::DeclRefExpr*>(BO->getLHS());

        std::string result;

        if(evalCompoundExprToInteger(result, BO)) {
            // If we can evaluate the expression to a string add the result
            // to the context map
            addToMap(LHS->getNameInfo().getAsString(), result, BO->getLocStart());
        } else {
            // Otherwise, mark the variable as UNRESOLVED after this point
            addToMap(LHS->getNameInfo().getAsString(), {}, BO->getLocStart());
        }
      }
    }
  }

  // Visit a function call
  // funciton calls only modify numeric types via pointers
  void VisitCallExpr(const clang::CallExpr* CE) {
    for(auto A : CE->arguments()) {
      auto base = A->IgnoreImpCasts();

      if(clang::Stmt::DeclRefExprClass == base->getStmtClass()) {
        auto DR = reinterpret_cast<const clang::DeclRefExpr*>(base);

        if(isPtrToIntegerType(DR->getType().getTypePtr())) {
          // If the variable is a int*, mark it as UNRESOLVED
          addToMap(DR->getNameInfo().getAsString(), {}, CE->getLocEnd());
        }
      }
    }
  }
};

} // namespace anonymous

class ConstantIntegerPropagatorImpl : public ConstantPropagator<IntegerVisitor> {
public:
  ConstantIntegerPropagatorImpl(const clang::CompilerInstance* ci)
    : ConstantPropagator<IntegerVisitor>(ci) {
  }
};

ConstantIntegerPropagator::ConstantIntegerPropagator(const clang::CompilerInstance* ci) {
  impl = new ConstantIntegerPropagatorImpl(ci);
}

ConstantIntegerPropagator::~ConstantIntegerPropagator() {
  delete impl;
}

PropagationResult<std::string> ConstantIntegerPropagator::runPropagation
(const clang::FunctionDecl* function, const clang::DeclRefExpr* variable) {
  return impl->runPropagation(function, variable);
}

void ConstantIntegerPropagator::dump(std::ostream& stream) const {
  impl->dump(stream);
}

} // namespace propagation
} // namespace clangmetatool

// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
