#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_FIND_CALLS_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/AST/Expr.h>

#include <clangmetatool/collectors/find_calls_data.h>

namespace clangmetatool {
  namespace collectors {

      class FindCallsImpl;

      class FindCalls {

      private:

        FindCallsImpl* impl;

      public:

        FindCalls( clang::CompilerInstance          *ci,
                   clang::ast_matchers::MatchFinder *f,
                   std::string                      *n,
                   unsigned int                     *a  );

        ~FindCalls();

        FindCallsData* getData();
       std::pair<bool, clang::APValue> try_to_evaluate(const clang::Expr*, const clang::DeclRefExpr*);

      };

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
