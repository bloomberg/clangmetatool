#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_INCLUDE_GRAPH_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_INCLUDE_GRAPH_DATA_H

#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <string>

#include <clangmetatool/types/macro_reference_info.h>
#include <clangmetatool/types/file_attribute_map.h>
#include <clangmetatool/types/file_attribute_multimap.h>
#include <clangmetatool/types/file_graph_edge_multimap.h>
#include <clangmetatool/types/file_graph.h>
#include <clangmetatool/types/file_uid.h>
#include <iosfwd>

namespace clangmetatool {
  namespace collectors {

    /**
     * The data collected by the IncludeGraph collector
     */
    struct IncludeGraphData {

      /**
       * Translate file uid to name (as used in the include statement)
       */
      clangmetatool::types::FileAttributeMap<std::string>   fuid2name;

      /**
       * Translate file uid to the FileEntry pointer.
       */
      clangmetatool::types::FileAttributeMap<
        const clang::FileEntry* >                           fuid2entry;

      /**
       * Does the given file uid belong to a system header?
       */
      clangmetatool::types::FileAttributeMap<bool>          is_system;

      /**
       * Captures "include next" statements, since they are addressed
       * by the same "name"
       */
      clangmetatool::types::FileAttributeMap<
        clangmetatool::types::FileUID >                     include_next;

      /**
       * The location of the last include statement on the file (by
       * file uid).
       */
      clangmetatool::types::FileAttributeMap<
        clang::SourceLocation >                             last_include;

      /**
       * file uid A includes file uid B
       */
      clangmetatool::types::FileGraph                       include_graph;

      /**
       * Where are the include statements
       */
      clangmetatool::types::FileGraphEdgeMultimap<
        clang::SourceRange>                                 include_statements;

      /**
       * file uid A uses file uid B
       */
      clangmetatool::types::FileGraph                       use_graph;

      /**
       * file uid A references macro defined in B
       */
      clangmetatool::types::FileGraphEdgeMultimap<
        clangmetatool::types::MacroReferenceInfo >          macro_references;

      /**
       * file uid A declares something canonically declared in B
       */
      clangmetatool::types::FileGraphEdgeMultimap<
        const clang::Decl* >                                redeclarations;

      /**
       * file uid A references something declared in B
       */
      clangmetatool::types::FileGraphEdgeMultimap<
        const clang::DeclRefExpr* >                         decl_references;

      /**
       * file uid A references a type declared in B
       */
      clangmetatool::types::FileGraphEdgeMultimap<
        const clang::TypeLoc* >                             type_references;

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
