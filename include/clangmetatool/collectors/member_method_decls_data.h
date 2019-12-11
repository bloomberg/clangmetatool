#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_MEMBER_METHOD_DECLS_DATA
#define INCLUDED_CLANGMETATOOL_COLLECTORS_MEMBER_METHOD_DECLS_DATA

#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <set>
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
 * The data collected by the MemberMethodDecls collector
 */
struct MemberMethodDeclsData {

  /**
   * Contains member method declarations.
   */
  std::set<const clang::CXXMethodDecl *> decls;
};
} // namespace collectors
} // namespace clangmetatool

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
