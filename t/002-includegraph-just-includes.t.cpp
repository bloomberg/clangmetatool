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

#include <tuple>

template<class... Args>
class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::IncludeGraph i;
  std::tuple<Args...> additionalArgs;
public:
  MyTool(clang::CompilerInstance* ci,
         clang::ast_matchers::MatchFinder *f,
         std::tuple<Args...>& args)
    :ci(ci), i(ci, f), additionalArgs(args) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    clangmetatool::collectors::IncludeGraphData *d = i.getData();

    ASSERT_EQ(3, d->include_graph.size())
      << "there are four files, three include statements";

    ASSERT_EQ(0, d->use_graph.size())
      << "there is no use, just include statements";
    ASSERT_EQ(0, d->macro_references.size())
      << "there is no use, just include statements";
    ASSERT_EQ(0, d->redeclarations.size())
      << "there is no use, just include statements";
    ASSERT_EQ(0, d->decl_references.size())
      << "there is no use, just include statements";
    ASSERT_EQ(0, d->type_references.size())
      << "there is no use, just include statements";

    // let's make sure we got the right data.
    clang::SourceManager &sm = ci->getSourceManager();
    const clang::FileEntry *mfe = sm.getFileEntryForID(sm.getMainFileID());

    clangmetatool::types::FileUID base = mfe->getUID();
    int count = 0;

    const char* names[] = { "foo.cpp", "a.h", "b.h", "c.h" };

    while (1) {
      clangmetatool::types::FileGraphEdge e = { base, 0 };
      clangmetatool::types::FileGraph::iterator it
        = d->include_graph.lower_bound(e);

      if (count < 3) {
        ASSERT_NE(it, d->include_graph.end())
          << "found an include statement on the file " << names[count];
        ASSERT_EQ(base, it->first)
          << "found an include statement on the file " << names[count];
    
        clangmetatool::types::FileUID other = it->second;

        it++;
        ASSERT_NE(base, it->first)
          << "should not have found another include statement " << names[count];

        ASSERT_EQ(std::string(names[count+1]), d->fuid2name[other])
          << names[count] << " includes " << names[count+1];

        base = other;
        count++;
      } else {
        ASSERT_EQ(it, d->include_graph.end())
          << "last file should not have an include " << names[count];
        break;
      }
    }
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/002-includegraph-just-includes/foo.cpp",
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
