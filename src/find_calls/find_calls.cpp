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

  class FindCallsImpl {
  private:
    std::string n;
    unsigned int a;
    FindCallsData data;
    clang::CompilerInstance *ci;

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

  public:
    FindCallsImpl
      (clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder* f, std::string* n, unsigned int* a)
      : ci(ci),n(*n),a(*a),
        cb1(ci, &data),
        cb2(ci, &data)
    {
      f->addMatcher(sm1, &cb1);
      f->addMatcher(sm2, &cb2);
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
