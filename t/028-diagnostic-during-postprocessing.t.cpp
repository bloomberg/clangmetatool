#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>

#include "clang/AST/RecursiveASTVisitor.h"
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

struct FindFirstSourceLocation
  : public clang::RecursiveASTVisitor<FindFirstSourceLocation> {
  clang::SourceLocation location;
  bool VisitDecl(clang::Decl* decl) {
    if (decl->getBeginLoc().isValid()) {
      location = decl->getBeginLoc();
      return false;
    }
    return true;
  }
};

class MyTool {
private:
  clang::CompilerInstance* ci;
  clang::ast_matchers::MatchFinder *f;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), f(f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    auto& DE = ci->getDiagnostics();
    const unsigned ID = DE.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                           "Beginning of translation unit");

    // Find the first real source location in AST
    FindFirstSourceLocation finder;
    finder.TraverseDecl(ci->getASTContext().getTranslationUnitDecl());

    // Make sure we don't crash when reporting a diagnostic at this location
    // during post-processing.
    DE.Report(finder.location, ID);
  }
};

TEST(diagnostic_during_postprocessing, test) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/028-diagnostic-during-postprocessing/test.cpp",
    "--",
    "-xc++"
  };

  clang::tooling::CommonOptionsParser
    optionsParser
    ( argc, argv,
      MyToolCategory );
  clang::tooling::RefactoringTool tool
    ( optionsParser.getCompilations(),
      optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements());

  int r = tool.runAndSave(&raf);
}


// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
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
