#include <algorithm>
#include <array>
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
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iosfwd>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <memory>
#include <ostream>
#include <sstream>
#include <unistd.h>
#include <utility>

#include <clangmetatool/collectors/include_graph.h>
#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/types/file_uid.h>

#include "find_decl_match_callback.h"
#include "find_decl_ref_match_callback.h"
#include "find_type_match_callback.h"
#include "include_finder.h"

namespace clangmetatool {
namespace collectors {

using namespace clang::ast_matchers;
class IncludeGraphImpl {
private:
  clang::CompilerInstance *ci;
  IncludeGraphData data;

  clang::ast_matchers::StatementMatcher sm1 =
      clang::ast_matchers::declRefExpr().bind("ref");
  clang::ast_matchers::TypeLocMatcher sm2 =
      clang::ast_matchers::typeLoc(optionally(loc((const internal::Matcher<clang::QualType> &)hasDeclaration(decl().bind("decl"))))).bind("type");
  clang::ast_matchers::DeclarationMatcher sm3 =
      clang::ast_matchers::decl().bind("decl");

  clangmetatool::collectors::include_graph::FindDeclRefMatchCallback cb1;
  clangmetatool::collectors::include_graph::FindTypeMatchCallback cb2;
  clangmetatool::collectors::include_graph::FindDeclMatchCallback cb3;

public:
  IncludeGraphImpl(clang::CompilerInstance *ci,
                   clang::ast_matchers::MatchFinder *f)
      : ci(ci), cb1(ci, &data), cb2(ci, &data), cb3(ci, &data) {

    f->addMatcher(sm1, &cb1);
    f->addMatcher(sm2, &cb2);
    f->addMatcher(sm3, &cb3);

    // preprocessor callbacks
    ci->getPreprocessor().addPPCallbacks(
        std::make_unique<
            clangmetatool::collectors::include_graph::IncludeFinder>(ci,
                                                                     &data));
  }

  ~IncludeGraphImpl() {}

  IncludeGraphData *getData() { return &data; }
};

IncludeGraph::IncludeGraph(clang::CompilerInstance *ci,
                           clang::ast_matchers::MatchFinder *f) {
  impl = new IncludeGraphImpl(ci, f);
}

IncludeGraph::~IncludeGraph() { delete impl; }

IncludeGraphData *IncludeGraph::getData() { return impl->getData(); }
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
