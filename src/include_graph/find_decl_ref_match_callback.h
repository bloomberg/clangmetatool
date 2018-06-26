#ifndef INCLUDED_FIND_DECL_REF_MATHC_CALLBACK_H
#define INCLUDED_FIND_DECL_REF_MATHC_CALLBACK_H

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>

#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_uid.h>

namespace clangmetatool {
  namespace collectors {
    namespace include_graph {

      using clangmetatool::collectors::IncludeGraphData;

      class FindDeclRefMatchCallback
        : public clang::ast_matchers::MatchFinder::MatchCallback {

      private:
        clang::CompilerInstance *ci;
        IncludeGraphData* data;

      public:
        FindDeclRefMatchCallback(clang::CompilerInstance *ci,
                                 IncludeGraphData* d)
          :ci(ci), data(d) {}

        virtual void
        run(const clang::ast_matchers::MatchFinder::MatchResult &r)
          override;
      };
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
