#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_INCLUDE_GRAPH_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_INCLUDE_GRAPH_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>

#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_uid.h>

namespace clangmetatool {
  namespace collectors {

    /**
     * forward declaration to implementation details of the
     * collector.
     */
    class IncludeGraphImpl;

    /**
     * Include Graph data collector. Collects the information related
     * to include statements as well as references between different
     * files in the translation unit.
     */
    class IncludeGraph {
    private:

      /**
       * Pointer to Implementation
       */
      IncludeGraphImpl* impl;

    public:

      /**
       * Explicit constructor, to allow for implementation details.
       */
      IncludeGraph( clang::CompilerInstance          *ci,
                    clang::ast_matchers::MatchFinder *f   );

      /**
       * Explicit destructor.
       */
      ~IncludeGraph();

      /**
       * Get the pointer to the data structure, populated or not.
       */
      IncludeGraphData* getData();

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
