#ifndef INCLUDED_INCLUDE_FINDER_H
#define INCLUDED_INCLUDE_FINDER_H

#include <clang/Basic/FileManager.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <llvm/ADT/StringRef.h>

#include <clangmetatool/types/file_uid.h>
#include <clangmetatool/collectors/include_graph_data.h>

// Required to know which version of LLVM/Clang we're building against
#include <llvm/Config/llvm-config.h>

namespace clangmetatool {
namespace collectors {
namespace include_graph {

using clangmetatool::collectors::IncludeGraphData;

class IncludeFinder : public clang::PPCallbacks {
private:
  clang::CompilerInstance *ci;
  IncludeGraphData *data;

public:
  IncludeFinder(clang::CompilerInstance *ci, IncludeGraphData *d)
      : ci(ci), data(d) {}

  virtual void
  InclusionDirective(clang::SourceLocation hashLoc,
                     const clang::Token &includeToken, llvm::StringRef filename,
                     bool isAngled, clang::CharSourceRange filenameRange,
                     const clang::FileEntry *file, llvm::StringRef searchPath,
                     llvm::StringRef relativePath,
                     const clang::Module *imported,
                     clang::SrcMgr::CharacteristicKind FileType_) override;

  virtual void MacroExpands(const clang::Token &macroUsage,
                            const clang::MacroDefinition &macroDef,
                            clang::SourceRange range,
                            const clang::MacroArgs *args) override;

  virtual void Defined(const clang::Token &macroUsage,
                       const clang::MacroDefinition &macroDef,
                       clang::SourceRange range) override;

  virtual void Ifdef(clang::SourceLocation loc, const clang::Token &macroUsage,
                     const clang::MacroDefinition &macroDef) override;

  virtual void Ifndef(clang::SourceLocation loc, const clang::Token &macroUsage,
                      const clang::MacroDefinition &macroDef) override;
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
