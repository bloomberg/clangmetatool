#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

bool constructor_called;
bool postprocessing_called;

class MyTool {
private:
  clang::CompilerInstance *ci;
  clang::ast_matchers::MatchFinder *f;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), f(f) {
    constructor_called = true;
  }
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    ASSERT_NE((void *)NULL, ci);
    ASSERT_NE((void *)NULL, f);
    postprocessing_called = true;
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {"foo", "/dev/null", "--", "-xc++"};

  clang::tooling::CommonOptionsParser optionsParser(argc, argv, MyToolCategory);
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  constructor_called = false;
  postprocessing_called = false;

  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>> raf(
      tool.getReplacements());

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
  ASSERT_EQ(true, constructor_called);
  ASSERT_EQ(true, postprocessing_called);
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
