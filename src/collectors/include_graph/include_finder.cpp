#include "include_finder.h"

#include <algorithm>
#include <array>
#include <iosfwd>
#include <limits.h>
#include <ostream>
#include <sstream>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utility>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/DirectoryLookup.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Tooling.h>

#if LLVM_VERSION_MAJOR >= 17
#include <optional>
#else
#include <llvm/ADT/Optional.h>
#endif

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>

#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/types/macro_reference_info.h>

#include "include_graph_util.h"

namespace clangmetatool {
namespace collectors {
namespace include_graph {

using clangmetatool::types::MacroReferenceInfo;

void IncludeFinder::InclusionDirective(
    clang::SourceLocation hashLoc, const clang::Token &includeToken,
    llvm::StringRef filename, bool isAngled,
#if LLVM_VERSION_MAJOR >= 17
    clang::CharSourceRange filenameRange, clang::OptionalFileEntryRef file,
#elif LLVM_VERSION_MAJOR >= 15
    clang::CharSourceRange filenameRange, llvm::Optional<clang::FileEntryRef> file,
#else
    clang::CharSourceRange filenameRange, const clang::FileEntry *file,
#endif
    llvm::StringRef searchPath, llvm::StringRef relativePath,
    const clang::Module *imported,
    clang::SrcMgr::CharacteristicKind FileType_) {

  // The filetype characteristic is unused for now, hence marked with
  // a trailing '_'. We are recording all filetypes
#if LLVM_VERSION_MAJOR >= 15
  if(file.has_value()) {
    add_include_statement(ci, data, hashLoc, includeToken, filename, isAngled,
                          filenameRange, &file.value().getFileEntry(),
                          searchPath, relativePath,
                          imported);
  }
#else
  add_include_statement(ci, data, hashLoc, includeToken, filename, isAngled,
                        filenameRange, file, searchPath, relativePath,
                        imported);
#endif
}

void IncludeFinder::MacroExpands(const clang::Token &macroUsage,
                                 const clang::MacroDefinition &macroDef,
                                 clang::SourceRange range,
                                 const clang::MacroArgs *args) {
  add_macro_reference(ci, data,
                      MacroReferenceInfo(macroUsage, macroDef, range, args));
}

void IncludeFinder::Defined(const clang::Token &macroUsage,
                            const clang::MacroDefinition &macroDef,
                            clang::SourceRange range) {
  add_macro_reference(ci, data,
                      MacroReferenceInfo(macroUsage, macroDef, range, NULL));
}

void IncludeFinder::Ifdef(clang::SourceLocation loc,
                          const clang::Token &macroUsage,
                          const clang::MacroDefinition &macroDef) {
  add_macro_reference(ci, data,
                      MacroReferenceInfo(macroUsage, macroDef,
                                         clang::SourceRange(loc, loc), NULL));
}

void IncludeFinder::Ifndef(clang::SourceLocation loc,
                           const clang::Token &macroUsage,
                           const clang::MacroDefinition &macroDef) {
  add_macro_reference(ci, data,
                      MacroReferenceInfo(macroUsage, macroDef,
                                         clang::SourceRange(loc, loc), NULL));
}
} // namespace include_graph
} // namespace collectors
} // namespace clangmetatool

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
