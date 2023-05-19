#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/collectors/include_graph.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance *ci;
  clangmetatool::collectors::IncludeGraph graph;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), graph(ci, f) {}
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    clangmetatool::collectors::IncludeGraphData *data = graph.getData();

    // file ID 0 to 1, aka foo.cpp to paste.h
    auto edge = std::make_pair(0, 1);
    ASSERT_EQ(data->usage_reference_count.count(edge), 1);
    ASSERT_EQ(data->usage_reference_count[edge], 2); // reference PASTE 2x

    // file ID from 0 to 2, aka foo.cpp to global.h
    edge = std::make_pair(0, 2);
    ASSERT_EQ(data->usage_reference_count.count(edge), 1);
    // see comments in foo.cpp about reference counts
    ASSERT_EQ(data->usage_reference_count[edge], 6);

    // file ID from 0 to 3, aka foo.cpp to macro.h
    edge = std::make_pair(0, 3);
    // ANOTHER_PASTE & REFERENCE_GLOBAL1
    ASSERT_EQ(data->usage_reference_count[edge], 2);

    // file ID from 0 to 5, aka foo.cpp to indirect.h
    edge = std::make_pair(0, 3);
    // ANOTHER_PASTE & REFERENCE_GLOBAL1
    ASSERT_EQ(data->usage_reference_count[edge], 2);

    // file ID from 1 to 2, aka paste.h to global.h
    edge = std::make_pair(1, 2);
    ASSERT_EQ(data->usage_reference_count.count(edge), 1);
    // the macro usages in foo.cpp have the reference counts
    ASSERT_EQ(data->usage_reference_count[edge], 0);

    // file ID from 3 to 2, aka macro.h to global.h
    edge = std::make_pair(3, 2);
    // The call to REFERENCE_GLOBAL3 in foo.cpp has spelling of GLOBAL3
    //  in macro.h
    ASSERT_EQ(data->usage_reference_count.count(edge), 1);
    ASSERT_EQ(data->usage_reference_count[edge], 1);

    // file ID from 4 to 2, aka indirect.h to global.h
    edge = std::make_pair(4, 2);
    ASSERT_EQ(data->usage_reference_count.count(edge), 0);
    ASSERT_EQ(data->usage_reference_count[edge], 0);

    // file ID from 4 to 1, aka indirect.h to paste.h
    edge = std::make_pair(4, 1);
    ASSERT_EQ(data->usage_reference_count.count(edge), 1);
    ASSERT_EQ(data->usage_reference_count[edge], 2);
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  const char *argv[] = {"foo",
                        CMAKE_SOURCE_DIR
                        "/t/data/043-variable-access-through-expansion/foo.cpp",
                        "--", "-xc++"};
  int argc = sizeof(argv) / sizeof(argv[0]);

  auto result = clang::tooling::CommonOptionsParser::create(
      argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser &optionsParser = result.get();

  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>> raf(
      tool.getReplacements());

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}

// ----------------------------------------------------------------------------
// Copyright 2023 Bloomberg Finance L.P.
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
