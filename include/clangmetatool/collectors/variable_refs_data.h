#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_VARIABLE_REFS_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_VARIABLE_REFS_DATA_H

#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <map>
#include <string>

#include <clangmetatool/types/file_attribute_map.h>
#include <clangmetatool/types/file_attribute_multimap.h>
#include <clangmetatool/types/file_graph.h>
#include <clangmetatool/types/file_graph_edge_multimap.h>
#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/types/macro_reference_info.h>
#include <iosfwd>

namespace clangmetatool {
namespace collectors {

/**
 * The data collected by the VariableRefs collector
 */
struct VariableRefsData {

  /**
   * List all the references for that valuedecl.
   */
  std::multimap<const clang::VarDecl *, const clang::DeclRefExpr *> refs;

  /**
   * List usages of that variable as rvalue.  You may actually be
   * interested in the usages that are *not* rvalues, but the way
   * the collector works requires us to do this indirection.
   */
  std::set<const clang::DeclRefExpr *> rvalue_refs;

  /**
   * List usages of that variable that are plain variable
   * assignment. The idea is that if the variable is never
   * initialized, and the only non-rvalue usage is a clear
   * assignment, then its value is deterministic.
   *
   * More elaborate execution-path analysis would be able to tell
   * this better, but there are situations where this will be
   * sufficient.
   */
  std::map<const clang::DeclRefExpr *, const clang::Expr *>
      clear_assignment_refs;
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
