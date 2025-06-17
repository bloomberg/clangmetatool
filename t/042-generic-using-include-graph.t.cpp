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

class WeakDependenciesTool {
private:
  clang::CompilerInstance *ci;
  clangmetatool::collectors::IncludeGraph includeGraph;

public:
  WeakDependenciesTool(clang::CompilerInstance *ci,
                       clang::ast_matchers::MatchFinder *f)
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
    EXPECT_EQ(
        clangmetatool::IncludeGraphDependencies::DirectDependenciesMap(
            {std::make_pair(
                 fname2uid["generic_using.h"],
                 std::set<clangmetatool::types::FileUID>{fname2uid["generic_using.h"]})}),
        clangmetatool::IncludeGraphDependencies::liveWeakDependencies(data,
                                                                      mfid));

    clangmetatool::IncludeGraphDependencies::decrementUsageRefCount(
        data, {mfid, fname2uid["generic_using.h"]});

    // generic_using.h no longer needed
    EXPECT_EQ(
        clangmetatool::IncludeGraphDependencies::DirectDependenciesMap({}),
        clangmetatool::IncludeGraphDependencies::liveWeakDependencies(data,
                                                                      mfid));

  }
};

TEST(include_validate_test, liveWeakDependencies) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {
      "foo", CMAKE_SOURCE_DIR "/t/data/042-generic-using-include-graph/foo.cpp",
      "--", "-xc++"};

  auto result = clang::tooling::CommonOptionsParser::create(
      argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser &optionsParser = result.get();

  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<WeakDependenciesTool>>
      raf(tool.getReplacements());

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}
