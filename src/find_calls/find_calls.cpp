#ifndef INCLUDED_FIND_CALLS_H
#define INCLUDED_FIND_CALLS_H

#include <string>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/find_calls_data.h>

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
          r.Nodes.getNodeAs<clang::CallExpr>("callee");

        const clang::FunctionDecl *f =
          r.Nodes.getNodeAs<clang::FunctionDecl>("caller");

        data->call_contexts.insert
          (std::pair
          <const clang::CallExpr*,
          const clang::FunctionDecl*>(c,f));
      }

  };

  class FindCallsImpl {
  private:
    std::string n;
    FindCallsData data;
    StatementMatcher sm =
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
              )
            )
          )
        ),
        hasAncestor(functionDecl().bind("caller"))
      ).bind("callee");
    AnnotateCall cc;

  public:
    FindCallsImpl
      (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string n)
      : cc(ci, &data), n(n)
    {
      f->addMatcher(sm, &cc);
    }

    ~FindCallsImpl() {
    }

    FindCallsData* getData() {
      return &data;
    }

  };

  FindCalls::FindCalls
  (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string n): n(n) {
    impl = new FindCallsImpl(ci, f, n);
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
