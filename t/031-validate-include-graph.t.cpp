#include "clangmetatool-testconfig.h"


#include <gtest/gtest.h>
#include <map>
#include <string>
#include <iostream>

#include <clangmetatool/collectors/include_graph.h>
#include <clangmetatool/collectors/include_graph_data.h>
#include <clangmetatool/include_graph_dependencies.h>
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
  clangmetatool::collectors::IncludeGraph includeGraph;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), includeGraph(ci, f) {}
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementMap) {
    clangmetatool::collectors::IncludeGraphData *data = includeGraph.getData();

    std::map<std::string, clangmetatool::types::FileUID> fname2uid;
    for (auto itr = data->fuid2name.begin(); itr != data->fuid2name.end();
         ++itr) {
      fname2uid[itr->second] = itr->first;
    }

    // Get main file UID
    clang::SourceManager &sm = ci->getSourceManager();
    const clang::FileEntry *mfe = sm.getFileEntryForID(sm.getMainFileID());
    clangmetatool::types::FileUID mfid = mfe->getUID();

    // clang-format: off
    // Symbol counts needed by foo:
    // [B: 1, C: 1, D1: 1, D2: 1, E: 1, F: 1]
    //
    // The include graph for foo looks like so, with parentheses containing
    // symbols that are provided by that header used by foo.cpp:
    //
    //         +-> a.h ()
    //         |
    //         +-> b.h -> def1.h (B)
    //         |
    // foo.cpp +-> c.h -> mid.h -> def2.h (C)
    //         |
    //         +-> d.h +-> def3.h (D1)
    //         |       |
    //         |       +-> def4.h (D2)
    //         |
    //         +-> e.h -----+--> def5.h (E)
    //         |            |
    //         +-> diam.h --+
    //         |
    //         +-> f.h -> def6.h (F)
    //         |    ^
    //         |    |
    //         +-> level.h
    //
    // It should be noted that foo.cpp:
    // * does not depend on a.h, as it provides no symbols used by foo
    // * Received 'B' through b.h
    // * Recieved 'C' through c.h
    // * Received 'D1', 'D2' through d.h
    // * Received 'E' through both e.h, diam.h
    // * Received 'F' through both f.h, level.h

    // foo.cpp: b.h c.h d.h e.h f.h
    // (a.h provides nothing, diam.h adds no new symbols)
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["b.h"],
          fname2uid["c.h"],
          fname2uid["d.h"],
          fname2uid["e.h"],
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    // Total refcount of def1.h is 0 now, foo no longer depends on b.h
    // foo.cpp: c.h d.h e.h f.h
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["def1.h"]}
    );
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["c.h"],
          fname2uid["d.h"],
          fname2uid["e.h"],
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    // Total refcount of def2.h is now 0, foo.cpp no longer depends on c.h
    // foo.cpp: d.h e.h f.h
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
      data, {mfid, fname2uid["def2.h"]}
    );
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["d.h"],
          fname2uid["e.h"],
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    // Total refcount for def3.h is now 0, foo.cpp will still depend on d.h
    // becuase it provides the only path to access 'D2' from def4.h
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["def3.h"]}
    );
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["d.h"],
          fname2uid["e.h"],
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    // Total refcount for def4.h is now 0, foo.cpp no longer depends on d.h
    // foo.cpp: e.h f.h
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["def4.h"]}
    );
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["e.h"],
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["def5.h"]}
    );
    // foo.cpp only depends on f.h now
    EXPECT_EQ(
        std::set<clangmetatool::types::FileUID>({
          fname2uid["f.h"]
        }),
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
    );

    // Total refcount of def6.h is 0 now, foo.cpp now longer depends on f.h
    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["def6.h"]}
    );
    // Dependency set of foo.h is empty
    EXPECT_TRUE(
        clangmetatool::IncludeGraphDependencies::liveDependencies(data, mfid)
        .empty());
  }
};

TEST(include_validate_test, basic) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {
      "foo", CMAKE_SOURCE_DIR "/t/data/031-validate-include-graph/foo.cpp",
      "--", "-xc++"};

  clang::tooling::CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>> raf(
      tool.getReplacements());

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}
