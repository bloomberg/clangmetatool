#ifndef INCLUDED_INCLUDE_GRAPH_UTIL_H
#define INCLUDED_INCLUDE_GRAPH_UTIL_H

#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Token.h>
#include <llvm/ADT/StringRef.h>

#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/types/macro_reference_info.h>

namespace clangmetatool {
namespace collectors {
namespace include_graph {

using clangmetatool::collectors::IncludeGraphData;

void add_include_statement(clang::CompilerInstance *ci, IncludeGraphData *data,
                           clang::SourceLocation hashLoc,
                           const clang::Token &includeToken,
                           llvm::StringRef filename, bool isAngled,
                           clang::CharSourceRange filenameRange,
                           const clang::FileEntry *file,
                           llvm::StringRef searchPath,
                           llvm::StringRef relativePath,
                           const clang::Module *imported);

void add_macro_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                         clangmetatool::types::MacroReferenceInfo m);

void add_redeclaration(clang::CompilerInstance *ci, IncludeGraphData *data,
                       const clang::Decl *n);

void add_decl_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                        const clang::DeclRefExpr *n);

void add_type_reference(clang::CompilerInstance *ci, IncludeGraphData *data,
                        const clang::TypeLoc *n, const clang::Decl* decl);
} // namespace include_graph
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
