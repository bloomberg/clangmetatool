#ifndef INCLUDED_FIND_CALLS_H
#define INCLUDED_FIND_CALLS_H

#include <string>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/find_calls_data.h>

namespace clangmetatool {
  namespace collectors {

  using namespace clang::ast_matchers;

  class AnnotateCall1
      : public clang::ast_matchers::MatchFinder::MatchCallback {
    private:
      clang::CompilerInstance *ci;
      FindCallsData *data;
    public:
      AnnotateCall1
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

        const clang::DeclRefExpr *a =
          r.Nodes.getNodeAs<clang::DeclRefExpr>("arg");

        data->call_context.insert
          (std::pair
          <const clang::FunctionDecl*,
          const clang::CallExpr*>(f,c));

        data->call_ref.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::DeclRefExpr*>(c,e));

        data->call_argref.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::DeclRefExpr*>(c,a));
      }

  };

  class AnnotateCall2
      : public clang::ast_matchers::MatchFinder::MatchCallback {
    private:
      clang::CompilerInstance *ci;
      FindCallsData *data;
    public:
      AnnotateCall2
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

        const clang::StringLiteral *a =
          r.Nodes.getNodeAs<clang::StringLiteral>("arg");

        data->call_context.insert
          (std::pair
          <const clang::FunctionDecl*,
          const clang::CallExpr*>(f,c));

        data->call_ref.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::DeclRefExpr*>(c,e));

        data->call_argstr.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::StringLiteral*>(c,a));
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
        hasArgument(a, declRefExpr().bind("arg")),
        hasAncestor(functionDecl().bind("context"))
      ).bind("call");
    AnnotateCall1 cb1;

    StatementMatcher sm2 =
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
        hasArgument(a, stringLiteral().bind("arg")),
        hasAncestor(functionDecl().bind("context"))
      ).bind("call");
    AnnotateCall2 cb2;

    DeclarationMatcher sm3 =
      varDecl
      (hasParent
       (declStmt().bind("stmt")
        )
       ).bind("var");
    AnnotateDeclStmts cb3;

    StatementMatcher sm4 =
      declStmt
      (has
       (varDecl().bind("var")
        ),
       hasParent
       (compoundStmt().bind("cmpd"))
       ).bind("stmt");
    AnnotateCompoundStmts cb4;

    StatementMatcher sm5 =
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
    AnnotateAssignmentVarDeclRefExpr cb5;

    DeclarationMatcher sm6 =
      varDecl
      (hasParent
       (decl().bind("decl")
        )
       ).bind("var");
    AnnotateNestedDecls cb6;

  public:
    FindCallsImpl
      (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string* n, unsigned int* a)
      : n(*n),a(*a),
        cb1(ci, &data),
        cb2(ci, &data),
        cb3(ci, &data),
        cb4(ci, &data),
        cb5(ci, &data),
        cb6(ci, &data)
    {
      f->addMatcher(sm1, &cb1);
      f->addMatcher(sm2, &cb2);
      f->addMatcher(sm3, &cb3);
      f->addMatcher(sm4, &cb4);
      f->addMatcher(sm5, &cb5);
      f->addMatcher(sm6, &cb6);
    }

    ~FindCallsImpl() {
    }

    FindCallsData* getData() {
      return &data;
    }

  };

  FindCalls::FindCalls
  (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string* n, unsigned int* a)
  {
    impl = new FindCallsImpl(ci, f, n, a);
  }

  FindCalls::~FindCalls() {
    delete impl;
  }

  FindCallsData* FindCalls::getData() {
    return impl->getData();
  }
  }
}
#endif
