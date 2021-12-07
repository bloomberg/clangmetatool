#include "clangmetatool-testconfig.h"

#include <algorithm>
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
  typedef std::map<unsigned int, std::string> ExpectedDataMap;

  clang::CompilerInstance* ci;
  clangmetatool::MatchForwarder mf;
  std::vector<clang::SourceRange> data;
  ExpectedDataMap expectedData;

  class CommentHandler : public clang::CommentHandler {
    ExpectedDataMap *expectedData;

  public:
    CommentHandler(ExpectedDataMap *expectedData)
      : expectedData(expectedData) {
    }

    virtual bool HandleComment(clang::Preprocessor& preprocessor,
			       clang::SourceRange comment) override {
      const clang::SourceManager& sourceManager =
	preprocessor.getSourceManager();
      unsigned int line = sourceManager.getSpellingLineNumber(
        comment.getBegin());
      std::string source = clang::Lexer::getSourceText(
        clang::CharSourceRange::getTokenRange(comment),
	sourceManager,
	preprocessor.getLangOpts()).str();
      source.erase(0, source.find_first_not_of("/ "));
      (*expectedData)[line] = source;
      return false;
    }
  };

  CommentHandler commentHandler;

  void handleDeclRefExpr(const MatchFinder::MatchResult& r) {
    data.push_back(
      r.Nodes.getNodeAs<clang::Stmt>("ref")->getSourceRange());
  }

public:
  MyTool(clang::CompilerInstance* ci, MatchFinder *f)
    : ci(ci), mf(f), commentHandler(&expectedData) {
    using namespace std::placeholders;
    StatementMatcher beginMatcher =
        declRefExpr(hasDeclaration(namedDecl(hasName("begin"))));
    StatementMatcher endMatcher =
        declRefExpr(hasDeclaration(namedDecl(hasName("end"))));
    StatementMatcher matcher =
        binaryOperator(hasLHS(ignoringParenImpCasts(beginMatcher)),
                       hasRHS(ignoringParenImpCasts(endMatcher))).bind("ref");
    mf.addMatcher(matcher, std::bind(&MyTool::handleDeclRefExpr, this, _1));

    ci->getPreprocessor().addCommentHandler(&commentHandler);
  }

  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    const clang::SourceManager& sourceManager = ci->getSourceManager();
    clang::Preprocessor& preprocessor = ci->getPreprocessor();

    for (auto match : data) {
      clang::CharSourceRange range =
        clangmetatool::SourceUtil::expandRangeIfValid(match,
                                                      sourceManager,
                                                      preprocessor);

      unsigned int line = sourceManager.getSpellingLineNumber(
        sourceManager.getExpansionLoc(match.getBegin()));

      ExpectedDataMap::iterator expectedIt = expectedData.find(line);
      if (expectedData.end() == expectedIt) {
	EXPECT_NE(expectedIt, expectedData.end())
	  << "line: " << line;
      }
      else {
	std::string source = clang::Lexer::getSourceText(
	  range,
	  sourceManager,
	  preprocessor.getLangOpts()).str();
	
	EXPECT_EQ(source, expectedIt->second)
	  << "line: " << line
	  << ", match: " << match.printToString(sourceManager)
	  << ", range: " << range.getAsRange().printToString(sourceManager);
      }
    }
  }
};

} // namespace anonymous

TEST(expandRangeIfValid, basic) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/041-expand-range-if-valid/foo.cpp",
    "--",
    "-xc++",
    "-Wno-unused-value"
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
