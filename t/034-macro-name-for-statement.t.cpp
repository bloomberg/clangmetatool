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
    std::vector<std::string> expected = {
      "",
      "VAR",
      "",
      "VAR",
      "",
      "VAR"
    };

    ASSERT_EQ(data.size(), expected.size());
    for (int i = 0; i < data.size(); ++i) {
      EXPECT_EQ(clangmetatool::SourceUtil::getMacroNameForStatement(
                    *data[i],
                    ci->getSourceManager()),
                expected[i]) << "i: " << i;
    }
  }
};

} // namespace anonymous

TEST(propagation_MacroConstantPropagation, basic) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/034-macro-name-for-statement/foo.cpp",
    "--",
    "-xc"
  };

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();

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
