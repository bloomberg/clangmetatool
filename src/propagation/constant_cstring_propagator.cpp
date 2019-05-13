#include "propagation_visitor.h"
#include "constant_propagator.h"

#include <clangmetatool/propagation/constant_cstring_propagator.h>

#include <string>

#include <clang/Analysis/CFG.h>
#include <clang/AST/Expr.h>

namespace clangmetatool {
namespace propagation {
namespace {

// Given an expression, determine if it is a char*
bool isCharPtrType(const clang::Expr *E) {
  auto QT = E->getType();

  if (QT.isNull() || !QT.getTypePtr()->isPointerType()) return false;

  auto QT2 = QT.getTypePtr()->getPointeeType();

  return !QT2.isNull() && QT2.getTypePtr()->isCharType();
}

// Utility class to visit the statements of a block and update the
// ValueContextMap in the process
class CStringVisitor : public PropagationVisitor<CStringVisitor, std::string> {
private:
  // Given an expression, try to evaluate it to a string result. Return
  // false if this is not possible
  bool evalExprToString(std::string &result, const clang::Expr *E) {
    // We only care about char types
    if (isCharPtrType(E)) {
      clang::Expr::EvalResult ER;

      if (E->isEvaluatable(context) && E->EvaluateAsRValue(ER, context)) {
        std::string r = ER.Val.getAsString(context, E->getType());

        // So long as the string is not null
        if ("0" != r) {
          // Clang returns results as &"actual-string"[0]
          result = r.substr(2, r.length() - 6);

          return true;
        }
      }
    }

    return false;
  }

public:
  // Use parent class's constructor
  using PropagationVisitor<CStringVisitor, std::string>::PropagationVisitor;

  // Visit a declaration of a variable
  void VisitDeclStmt(const clang::DeclStmt *DS) {
    for (auto D : DS->decls()) {
      if (clang::Decl::Var == D->getKind()) {
        auto VD = reinterpret_cast<const clang::VarDecl *>(D);

        // Only the local non-static variables are possibly deterministic
        if (VD->isLocalVarDecl() && VD->hasLocalStorage()) {
          if (VD->hasInit()) {
            auto I = VD->getInit();

            std::string result;

            if (evalExprToString(result, I)) {
              // If the variable is a string, add it to the map
              addToMap(VD->getNameAsString(), result, VD->getBeginLoc());
            }
          }
        }
      }
    }
  }

  // Visit a binary operator
  void VisitBinaryOperator(const clang::BinaryOperator *BO) {
    // We only care about assignments (=) in this context
    if (clang::BO_Assign == BO->getOpcode()) {
      if (clang::Stmt::DeclRefExprClass == BO->getLHS()->getStmtClass()) {
        auto LHS = reinterpret_cast<const clang::DeclRefExpr *>(BO->getLHS());

        std::string result;

        if (evalExprToString(result, BO->getRHS())) {
          // If we can evaluate the expression to a string add the result
          // to the context map
          addToMap(LHS->getNameInfo().getAsString(), result, BO->getBeginLoc());
        } else {
          // Otherwise, mark the variable as UNRESOLVED after this point
          addToMap(LHS->getNameInfo().getAsString(), {}, BO->getBeginLoc());
        }
      }
    }
  }

  // Visit a function call
  void VisitCallExpr(const clang::CallExpr *CE) {
    for (auto A : CE->arguments()) {
      auto base = A->IgnoreImpCasts();

      if (clang::Stmt::DeclRefExprClass == base->getStmtClass()) {
        auto DR = reinterpret_cast<const clang::DeclRefExpr *>(base);

        if (isCharPtrType(DR)) {
          // If the variable is a char*, mark it as UNRESOLVED
          addToMap(DR->getNameInfo().getAsString(), {}, CE->getEndLoc());
        }
      }
    }
  }
};

} // namespace anonymous

class ConstantCStringPropagatorImpl
    : public ConstantPropagator<CStringVisitor> {
public:
  ConstantCStringPropagatorImpl(const clang::CompilerInstance *ci)
      : ConstantPropagator<CStringVisitor>(ci) {}
};

ConstantCStringPropagator::ConstantCStringPropagator(
    const clang::CompilerInstance *ci) {
  impl = new ConstantCStringPropagatorImpl(ci);
}

ConstantCStringPropagator::~ConstantCStringPropagator() { delete impl; }

PropagationResult<std::string>
ConstantCStringPropagator::runPropagation(const clang::FunctionDecl *function,
                                          const clang::DeclRefExpr *variable) {
  return impl->runPropagation(function, variable);
}

void ConstantCStringPropagator::dump(std::ostream &stream) const {
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
