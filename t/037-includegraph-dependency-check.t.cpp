#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/include_graph.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::IncludeGraph i;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), i(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    clangmetatool::collectors::IncludeGraphData *data = i.getData();

    ASSERT_NE(data->include_next.size(), 0);

    for (const auto &uid : data->include_next) {
      ASSERT_NE(uid.first, 1);
      ASSERT_NE(uid.second, 1);
      ASSERT_NE(uid.first, 0);
      ASSERT_NE(uid.second, 0);
    }
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/037-includegraph-dependency-check/tester.cpp",
    "--",
    "-xc++"
  };
  int argc = sizeof(argv) / sizeof(argv[0]);

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();

  clang::tooling::RefactoringTool tool
    ( optionsParser.getCompilations(),
      optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements());

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}


// ----------------------------------------------------------------------------
// Copyright 2021 Bloomberg Finance L.P.
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
