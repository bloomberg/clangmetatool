#include <clangmetatool/collectors/variable_refs.h>

namespace clangmetatool {
  namespace collectors {

    using namespace clang::ast_matchers;

    namespace {

      class AnnotateVarDeclRefExpr
        : public clang::ast_matchers::MatchFinder::MatchCallback {
      private:
        clang::CompilerInstance *ci;
        VariableRefsData *data;
      public:
        AnnotateVarDeclRefExpr
        (clang::CompilerInstance* ci,
         VariableRefsData *data)
          :ci(ci), data(data) {}

        virtual void
        run(const clang::ast_matchers::MatchFinder::MatchResult &r)
        override {

          const clang::DeclRefExpr *e =
            r.Nodes.getNodeAs<clang::DeclRefExpr>("ref");
          if (e == NULL) return;

          const clang::VarDecl *v =
            r.Nodes.getNodeAs<clang::VarDecl>("var");
          if (v == NULL) return;

          data->refs.insert
            (std::pair
             <const clang::VarDecl*,
             const clang::DeclRefExpr*>(v,e));

        }

      };

      class AnnotateRValueVarDeclRefExpr
        : public clang::ast_matchers::MatchFinder::MatchCallback {
      private:
        clang::CompilerInstance *ci;
        VariableRefsData *data;
      public:
        AnnotateRValueVarDeclRefExpr
        (clang::CompilerInstance* ci,
         VariableRefsData *data)
          :ci(ci), data(data) {}

        virtual void
        run(const clang::ast_matchers::MatchFinder::MatchResult &r)
        override {

          const clang::ImplicitCastExpr *c =
            r.Nodes.getNodeAs<clang::ImplicitCastExpr>("cast");
          if (c == NULL) return;

          clang::CastKind ck = c->getCastKind();
          if (ck != clang::CastKind::CK_LValueToRValue)
            return;

          const clang::DeclRefExpr *e =
            r.Nodes.getNodeAs<clang::DeclRefExpr>("ref");
          if (e == NULL) return;

          data->rvalue_refs.insert(e);

        }

      };

      class AnnotateAssignmentVarDeclRefExpr
        : public clang::ast_matchers::MatchFinder::MatchCallback {
      private:
        clang::CompilerInstance *ci;
        VariableRefsData *data;
      public:
        AnnotateAssignmentVarDeclRefExpr
        (clang::CompilerInstance* ci,
         VariableRefsData *data)
          :ci(ci), data(data) {}

        virtual void
        run(const clang::ast_matchers::MatchFinder::MatchResult &r)
        override {

          const clang::DeclRefExpr *d =
            r.Nodes.getNodeAs<clang::DeclRefExpr>("ref");
          if (d == NULL) return;

          const clang::Expr *e =
            r.Nodes.getNodeAs<clang::Expr>("expr");
          if (e == NULL) return;

          data->clear_assignment_refs.insert
            (std::pair
             <const clang::DeclRefExpr*,
             const clang::Expr*>(d,e));

        }

      };

    }

    class VariableRefsImpl {
    private:
      clang::CompilerInstance *ci;
      VariableRefsData data;

      StatementMatcher sm1 =
        declRefExpr
        (hasDeclaration
         (varDecl().bind("var")
          )
         ).bind("ref");
      AnnotateVarDeclRefExpr cb1;

      StatementMatcher sm2 =
        implicitCastExpr
        (has
         (declRefExpr
          (hasDeclaration
           (varDecl()
            )
           ).bind("ref"))
         ).bind("cast");
      AnnotateRValueVarDeclRefExpr cb2;

      StatementMatcher sm3 =
        binaryOperator
        (
         hasOperatorName("="),
         hasLHS
         (
          declRefExpr
          (hasDeclaration
           (varDecl()
            )
           ).bind("ref")
          ),
         hasRHS
         (
          expr().bind("expr")
          )
         );
      AnnotateAssignmentVarDeclRefExpr cb3;

    public:
      VariableRefsImpl
      (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f)
        :cb1(ci, &data), cb2(ci, &data), cb3(ci, &data) {
        f->addMatcher(sm1, &cb1);
        f->addMatcher(sm2, &cb2);
        f->addMatcher(sm3, &cb3);
      }
      VariableRefsData* getData() {
        return &data;
      }
    };

    VariableRefs::VariableRefs
    (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f) {
      impl = new VariableRefsImpl(ci, f);
    }

    VariableRefs::~VariableRefs() {
      delete impl;
    }

    VariableRefsData* VariableRefs::getData() {
      return impl->getData();
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
