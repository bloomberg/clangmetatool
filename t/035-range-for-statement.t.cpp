#include "clangmetatool-testconfig.h"

#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Basic/SourceManager.h>

#include <clangmetatool/match_forwarder.h>
#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/source_util.h>

#include <gtest/gtest.h>

namespace {

using namespace clang::ast_matchers;

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::MatchForwarder mf;
  std::vector<const clang::DeclRefExpr*> data;

  void handleDeclRefExpr(const MatchFinder::MatchResult& r) {
    data.push_back(r.Nodes.getNodeAs<clang::DeclRefExpr>("ref"));
  }

public:
  MyTool(clang::CompilerInstance* ci, MatchFinder *f)
    : ci(ci), mf(f) {
    using namespace std::placeholders;
    StatementMatcher matcher =
        declRefExpr(hasDeclaration(namedDecl(hasName("var")))).bind("ref");
    mf.addMatcher(matcher, std::bind(&MyTool::handleDeclRefExpr, this, _1));
  }

  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    const clang::SourceManager& sourceManager = ci->getSourceManager();

    struct Location {
      int line;
      int col;
    };

    std::vector<std::pair<Location, Location> > expected = {
      { {11, 3},  {11, 6} },
      { {12, 3},  {12, 6} },
      { {13, 8},  {13, 11} },
      { {14, 8},  {14, 11} },
      { {15, 3},  {15, 11} },
      { {16, 3},  {16, 11} },
      { {17, 14}, {17, 17} },
      { {18, 14}, {18, 17} },
      { {19, 9},  {19, 23} },
      { {20, 9},  {20, 23} }
    };

    ASSERT_EQ(data.size(), expected.size());
    for (int i = 0; i < data.size(); ++i) {
      clang::CharSourceRange range =
        clangmetatool::SourceUtil::getRangeForStatement(*data[i],
                                                        sourceManager);
      EXPECT_EQ(expected[i].first.line,
                sourceManager.getSpellingLineNumber(range.getBegin()))
        << "i: " << i;
      EXPECT_EQ(expected[i].first.col,
                sourceManager.getSpellingColumnNumber(range.getBegin()))
        << "i: " << i;
      EXPECT_EQ(expected[i].second.line,
                sourceManager.getSpellingLineNumber(range.getEnd()))
        << "i: " << i;
      EXPECT_EQ(expected[i].second.col,
                sourceManager.getSpellingColumnNumber(range.getEnd()))
        << "i: " << i;
    }
  }
};

} // namespace anonymous

TEST(propagation_MacroConstantPropagation, basic) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/035-range-for-statement/foo.cpp",
    "--",
    "-xc",
    "-Wno-unused-value"
  };
  clang::tooling::CommonOptionsParser optionsParser
    (argc, argv, MyToolCategory);
  clang::tooling::RefactoringTool tool
    (optionsParser.getCompilations(), optionsParser.getSourcePathList());
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>>
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
