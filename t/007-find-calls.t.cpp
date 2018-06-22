#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/find_calls.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::FindCalls fc;
  const std::string n = "bar";
public:

  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), fc(ci, f, n) {
  }
      void postProcessing
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
      clang::SourceManager& sm = ci->getSourceManager();
      std::map<const clang::CallExpr*, const clang::FunctionDecl*> *call_contexts = &(fc.getData()->call_contexts);
      auto ccit = call_contexts->begin();
      const clang::CallExpr* callee = ccit->first;
      const clang::FunctionDecl* context = ccit->second;
      const clang::FunctionDecl* function2 = callee->getDirectCallee();
      ASSERT_EQ(std::string("foo"), context->getNameAsString());
      ASSERT_EQ(2, callee->getNumArgs());
      ASSERT_EQ(std::string("bar"), function2->getNameAsString());

//TODO
// the bind must be:
//      1. to the body which calls the legacy function (which is parameterised)
//      2. the parameters (parameterised index) of the function call possibly yielding evaluation.
      }
  };

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/007-find-calls/foo.cpp",
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
  ASSERT_EQ(0, r);
}


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
