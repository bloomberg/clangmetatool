#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/include_graph.h>
#include <clangmetatool/include_graph_dependencies.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::IncludeGraph graph;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), graph(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    clangmetatool::collectors::IncludeGraphData *data = graph.getData();

    clangmetatool::types::FileUID header = 0;
    clangmetatool::types::FileUID cpp = 0;
    for(auto itr = data->fuid2name.cbegin(); itr != data->fuid2name.cend(); ++itr){
      if(itr->second == "foo.h"){
        header = itr->first;
      }
      if(itr->second == "foo.cpp"){
        cpp = itr->first;
      }
    }
    auto edge = std::make_pair(cpp, header);
    size_t edge_rc = 4; // edge initial reference count

    // validator that validate the edge and its reference count
    // before and after doing decrementUsageRefCount
    auto validator = [&](){
      ASSERT_EQ(data->usage_reference_count.count(edge), 1);
      ASSERT_EQ(data->usage_reference_count[edge], edge_rc);
      clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(data, edge);
      ASSERT_EQ(data->usage_reference_count[edge], edge_rc - 1);
      clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(data, edge);
      ASSERT_EQ(data->usage_reference_count[edge], edge_rc - 2);
    };

    // backup and validate the initial state 
    clangmetatool::IncludeGraphDependencies::backup(data);
    validator();
    
    // validate that the include graph is able to be restored
    clangmetatool::IncludeGraphDependencies::restore(data);
    validator();
    
    // validate that the include graph is able to be backup at a specific state
    // and restored back to that state
    clangmetatool::IncludeGraphDependencies::restore(data);
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(data, edge);
    edge_rc = 3;
    clangmetatool::IncludeGraphDependencies::backup(data);
    validator();
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/042-includegraph-reset-state/foo.cpp",
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
