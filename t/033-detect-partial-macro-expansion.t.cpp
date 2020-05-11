#include "clangmetatool-testconfig.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>

#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/source_util.h>

#include <functional>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace {
using namespace clang::ast_matchers;

// Custom matcher for the type of variable we're looking for:
// i.e. one that has global storage
auto globalArrayRefWithName = [](const std::string &name) -> StatementMatcher {
  return arraySubscriptExpr(
             hasBase(ignoringParenImpCasts(
                 declRefExpr(to(varDecl(hasGlobalStorage(), hasName(name))))
                     .bind("base"))))
      .bind("context");
};

} // namespace

class CollectDeclRef : public MatchFinder::MatchCallback {
public:
  std::set<const clang::DeclRefExpr *> d_refs;
  CollectDeclRef() : d_refs() {}

  virtual void run(const MatchFinder::MatchResult &result) override {
    if (const auto *ref =
            result.Nodes.getNodeAs<clang::DeclRefExpr>("context")) {
      d_refs.emplace(ref);
    }
  }
};

class TestTool {
public:
  typedef bool ArgTypes;

private:
  clang::CompilerInstance *d_ci;
  StatementMatcher d_globalVariableRef;
  std::unique_ptr<CollectDeclRef> d_callback;
  ArgTypes d_isPartialMacroFlag;

public:
  TestTool(clang::CompilerInstance *ci, MatchFinder *f,
           ArgTypes isPartialMacroFlag)
      : d_ci(ci), d_globalVariableRef(globalArrayRefWithName("_GLOBALARRAY")),
        d_callback(new CollectDeclRef()),
        d_isPartialMacroFlag(isPartialMacroFlag) {
    f->addMatcher(d_globalVariableRef, d_callback.get());
  }

  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    // These were all the refs to the variable we declared
    for (auto expr : d_callback->d_refs) {
      if (d_isPartialMacroFlag) {
        ASSERT_TRUE(clangmetatool::SourceUtil::isPartialMacro(
            expr->getSourceRange(), d_ci->getSourceManager(),
            d_ci->getPreprocessor()));
      } else {
        ASSERT_FALSE(clangmetatool::SourceUtil::isPartialMacro(
            expr->getSourceRange(), d_ci->getSourceManager(),
            d_ci->getPreprocessor()));
      }
    }
  }
};

TEST(partialMacroExpansion_Detect, positive) {
  llvm::cl::OptionCategory category("my-tool options");
  int argc = 4;
  const char *argv[] = {
      "foo",
      CMAKE_SOURCE_DIR
      "/t/data/033-detect-partial-macro-expansion/not-partial-macro-usages.cpp",
      "--", "-xc"};

  clang::tooling::CommonOptionsParser optionsParser(argc, argv, category);
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  bool expectedResult = true;
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<TestTool>> raf(
      tool.getReplacements(), expectedResult);
  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}

TEST(partialMacroExpansion_Detect, negative) {
  llvm::cl::OptionCategory category("my-tool options");
  int argc = 4;
  const char *argv[] = {
      "foo",
      CMAKE_SOURCE_DIR
      "/t/data/033-detect-partial-macro-expansion/partial-macro-usages.cpp",
      "--", "-xc"};

  clang::tooling::CommonOptionsParser optionsParser(argc, argv, category);
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  bool expectedResult = false;
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<TestTool>> raf(
      tool.getReplacements(), expectedResult);
  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}
