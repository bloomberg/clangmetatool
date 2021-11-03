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

    clangmetatool::collectors::IncludeGraphData *d = i.getData();

    ASSERT_EQ(1, d->include_graph.size())
      << "there are two files, one include statements";
    ASSERT_EQ(1, d->use_graph.size())
      << "there is one use";

    ASSERT_EQ(0, d->macro_references.size())
      << "there is no macro references";
    ASSERT_EQ(0, d->redeclarations.size())
      << "there is no redeclarations";
    ASSERT_EQ(0, d->type_references.size())
      << "there is no type references";

    ASSERT_EQ(1, d->decl_references.size())
      << "there is one decl reference";

    clangmetatool::types::FileGraph::iterator it =
      d->include_graph.begin();
    clangmetatool::types::FileGraphEdge e = *it;

    clangmetatool::types::FileGraphEdgeMultimap<const clang::DeclRefExpr*>
      ::iterator mmit =
      d->decl_references.lower_bound(e);

    ASSERT_NE(d->decl_references.end(), mmit)
      << "Found relationships in the decl_references";

    ASSERT_EQ(e, mmit->first)
      << "Found the same relationship in the decl_references";

    const clang::DeclRefExpr* declref = mmit->second;

    ASSERT_NE((void*)NULL, declref)
      << "declref is not NULL";

    ASSERT_EQ(std::string("somefunction"),
              declref->getNameInfo().getName().getAsString())
      << "is the right reference";
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/003-includegraph-function-ref/foo.cpp",
    "--",
    "-xc++"
  };

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
