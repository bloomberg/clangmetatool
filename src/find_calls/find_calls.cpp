#include <string>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/find_calls_data.h>
#include <clangmetatool/collectors/variable_refs.h>

namespace clangmetatool {
  namespace collectors {

  using namespace clang::ast_matchers;

  class AnnotateCall
      : public clang::ast_matchers::MatchFinder::MatchCallback {
    private:
      clang::CompilerInstance *ci;
      FindCallsData *data;
    public:
      AnnotateCall
      (clang::CompilerInstance* ci,
       FindCallsData *data)
        :ci(ci), data(data) {}

      virtual void
      run(const clang::ast_matchers::MatchFinder::MatchResult &r)
        override {

        const clang::CallExpr *c =
          r.Nodes.getNodeAs<clang::CallExpr>("call");

        const clang::FunctionDecl *f =
          r.Nodes.getNodeAs<clang::FunctionDecl>("context");

        const clang::DeclRefExpr *e =
          r.Nodes.getNodeAs<clang::DeclRefExpr>("ref");

        data->call_context.insert
          (std::pair
          <const clang::FunctionDecl*,
          const clang::CallExpr*>(f,c));

        data->call_ref.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::DeclRefExpr*>(c,e));

      }

  };

  class AnnotateDeclStmts
    : public clang::ast_matchers::MatchFinder::MatchCallback {
  private:
    clang::CompilerInstance *ci;
    FindCallsData *data;
  public:
    AnnotateDeclStmts
    (clang::CompilerInstance* ci,
     FindCallsData *data)
      :ci(ci), data(data) {}

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &r)
      override {

      const clang::DeclStmt *d =
        r.Nodes.getNodeAs<clang::DeclStmt>("stmt");
      if (d == NULL) return;

      const clang::VarDecl *v =
        r.Nodes.getNodeAs<clang::VarDecl>("var");
      if (v == NULL) return;

      data->declstmt_for_vardecl.insert
        (std::pair
         <const clang::VarDecl*,
         const clang::DeclStmt*>(v,d));

    }

  };

  class AnnotateCompoundStmts
    : public clang::ast_matchers::MatchFinder::MatchCallback {
  private:
    clang::CompilerInstance *ci;
    FindCallsData *data;
  public:
    AnnotateCompoundStmts
    (clang::CompilerInstance* ci,
     FindCallsData *data)
      :ci(ci), data(data) {}

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &r)
      override {

      const clang::DeclStmt *d =
        r.Nodes.getNodeAs<clang::DeclStmt>("stmt");
      if (d == NULL) return;

      const clang::CompoundStmt *c =
        r.Nodes.getNodeAs<clang::CompoundStmt>("cmpd");
      if (c == NULL) return;

      data->compoundstmt_for_declstmt.insert
        (std::pair
         <const clang::DeclStmt*,
         const clang::CompoundStmt*>(d,c));

    }

  };

  class AnnotateAssignmentVarDeclRefExpr
    : public clang::ast_matchers::MatchFinder::MatchCallback {
  private:
    clang::CompilerInstance *ci;
    FindCallsData *data;
  public:
    AnnotateAssignmentVarDeclRefExpr
    (clang::CompilerInstance* ci,
     FindCallsData *data)
      :ci(ci), data(data) {}

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &r)
      override {

      const clang::DeclRefExpr *d =
        r.Nodes.getNodeAs<clang::DeclRefExpr>("ref");
      if (d == NULL) return;

      const clang::BinaryOperator *a =
        r.Nodes.getNodeAs<clang::BinaryOperator>("assign");
      if (a == NULL) return;

      const clang::CompoundStmt *c =
        r.Nodes.getNodeAs<clang::CompoundStmt>("cmpd");
      if (c == NULL) return;

      data->binaryoperator_for_declrefexpr.insert
        (std::pair
         <const clang::DeclRefExpr*,
         const clang::BinaryOperator*>(d,a));

      data->compoundstmt_for_binaryoperator.insert
        (std::pair
         <const clang::BinaryOperator*,
         const clang::CompoundStmt*>(a,c));

    }

  };

  class AnnotateNestedDecls
    : public clang::ast_matchers::MatchFinder::MatchCallback {
  private:
    clang::CompilerInstance *ci;
    FindCallsData *data;
  public:
    AnnotateNestedDecls
    (clang::CompilerInstance* ci,
     FindCallsData *data)
      :ci(ci), data(data) {}

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &r)
      override {

      const clang::VarDecl *v =
        r.Nodes.getNodeAs<clang::VarDecl>("var");
      if (v == NULL) return;

      const clang::Decl *p =
        r.Nodes.getNodeAs<clang::Decl>("decl");
      if (p == NULL) return;

      data->nested_decls.insert
        (std::pair
        <const clang::VarDecl*,
         const clang::Decl*>(v,p));

    }

  };

  class FindCallsImpl {
  private:
    std::string n;
    unsigned int a;
    FindCallsData data;
    clang::CompilerInstance *ci;
    VariableRefs vr;

    StatementMatcher sm1 =
      callExpr(
        callee(
          implicitCastExpr(
            has(
              declRefExpr(
                hasDeclaration(
                  functionDecl(
                    hasAnyName(
                      n
                    )
                  )
                )
              ).bind("ref")
            )
          )
        ),
        hasAncestor(functionDecl().bind("context"))
      ).bind("call");
    AnnotateCall cb1;

    DeclarationMatcher sm2 =
      varDecl
      (hasParent
       (declStmt().bind("stmt")
        )
       ).bind("var");
    AnnotateDeclStmts cb2;

    StatementMatcher sm3 =
      declStmt
      (has
       (varDecl().bind("var")
        ),
       hasParent
       (compoundStmt().bind("cmpd"))
       ).bind("stmt");
    AnnotateCompoundStmts cb3;

    StatementMatcher sm4 =
      binaryOperator
      (hasOperatorName("="),
       hasLHS
       (declRefExpr
        (hasDeclaration
         (varDecl()
          )
         ).bind("ref")
        ),
       hasRHS
       (expr()
        ),
       hasParent
       (compoundStmt().bind("cmpd"))
       ).bind("assign");
    AnnotateAssignmentVarDeclRefExpr cb4;

    DeclarationMatcher sm5 =
      varDecl
      (hasParent
       (decl().bind("decl")
        )
       ).bind("var");
    AnnotateNestedDecls cb5;

  public:
    FindCallsImpl
      (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string n, unsigned int a)
      : ci(ci),n(n),a(a),vr(ci,f),
        cb1(ci, &data),
        cb2(ci, &data),
        cb3(ci, &data),
        cb4(ci, &data),
        cb5(ci, &data)
    {
      f->addMatcher(sm1, &cb1);
      f->addMatcher(sm2, &cb2);
      f->addMatcher(sm3, &cb3);
      f->addMatcher(sm4, &cb4);
      f->addMatcher(sm5, &cb5);
    }

    ~FindCallsImpl() {
    }

    FindCallsData* getData() {
      return &data;
    }

//    void insert_gen_call(GenCall call);

//    void insert_used_var(const clang::DeclRefExpr* var);

    std::pair<bool, clang::APValue>
    try_to_evaluate(const clang::Expr* expr,
                    const clang::DeclRefExpr* declrefexpr) {
        std::pair<bool, clang::APValue> ret(false, clang::APValue(0));

      // if we know the expr is a declrefexpr, let's just use that.
      if (declrefexpr != NULL)
        expr = declrefexpr;

      if (expr == NULL)
        return ret;

      clang::Expr::EvalResult eval_r;
      bool eval = expr->EvaluateAsRValue(eval_r, ci->getASTContext());

      if (eval) {
        ret.first = true;
        ret.second = eval_r.Val;
        return ret;

      } else if (declrefexpr != NULL) {
        VariableRefsData *data = vr.getData();

        // shoot, the input isn't a const. But not all is lost. We can
        // still handle cases where the variable has a deterministic
        // value.
        const clang::ValueDecl* dre_v = declrefexpr->getDecl();
        const clang::VarDecl*   var =
          llvm::dyn_cast_or_null<clang::VarDecl>(dre_v);
        if (var != NULL) {
          const clang::Expr* init = var->getAnyInitializer();

          bool has_init = false;
          bool has_assign = false;

          if (init != NULL) {
            has_init = true;
            eval = init->EvaluateAsRValue(eval_r, ci->getASTContext());
            if (!eval)
              return ret;
            ret.second = eval_r.Val;
          }

          // now we need to look at all the declrefexpr for this
          // variable and see if there are clear assignments, if
          // they're used as rvalues or if they're arguments to
          // figlbit calls. If everything goes well, we'd be able to
          // find a deterministic value.

//          auto vit = data->refs.lower_bound(var);
//          while (vit != data->refs.end() &&
//                 vit->first == var) {
//            const clang::DeclRefExpr* usage = vit->second;
//            auto aeit = data->clear_assignment_refs.find(usage);
//            if (data->rvalue_refs.find(usage) != data->rvalue_refs.end()) {
//              // this is a rvalue usage, not interesting...
//            } else if (aeit != data->clear_assignment_refs.end()) {
//              // this is a clear assignment.
//              if (has_assign) {
//                // second assignment, not deterministic.
//                return ret;
//              }
//              if (has_init) {
//                // initializer then assignment, not deterministic.
//                return ret;
//              }
//              eval = aeit->second->EvaluateAsRValue(eval_r, ci->getASTContext());
//              if (!eval) {
//                // expr is not constant, not determistic
//                return ret;
//              }
//              ret.second = eval_r.Val;
//              has_assign = true;
//            } else if (var_used_in_call.find(usage) !=
//                       var_used_in_call.end()) {
//              // this is used in fglblchk call, so we know it's a
//              // "rvalue" even if would be a pointer.
//            } else {
//              // a usage that we can't identify, so we can't consider
//              // the variable as having a deterministic value.
//              return ret;
//            }
//            vit++;
//          }
//
//          if (has_assign || has_init) {
//            ret.first = true;
//          }
        }
      }
      return ret;
    }

  };

  FindCalls::FindCalls
  (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string n, unsigned int a)
  {
    impl = new FindCallsImpl(ci, f, n, a);
  }

  FindCalls::~FindCalls() {
    delete impl;
  }

  FindCallsData* FindCalls::getData() {
    return impl->getData();
  }

  std::pair<bool, clang::APValue> FindCalls::try_to_evaluate(const clang::Expr* expr, const clang::DeclRefExpr* declrefexpr) {
      return impl->try_to_evaluate(expr, declrefexpr);
  }
  }
}
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
