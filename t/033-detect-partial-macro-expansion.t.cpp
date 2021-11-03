#include "clangmetatool-testconfig.h"

#include <llvm/Support/raw_ostream.h>

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

#include <exception>
#include <functional>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace {
using namespace clang::ast_matchers;

// Custom matcher for the type of variable we're looking for:
// i.e. one that has global storage
auto globalArrayRefWithName = [](const auto &name, const auto &parentName) {
  return arraySubscriptExpr(
             hasBase(ignoringParenImpCasts(
                 declRefExpr(to(varDecl(hasGlobalStorage(), hasName(name))))
                     .bind("base"))),
             hasAncestor(functionDecl(hasName(parentName))))
      .bind("context");
};

} // namespace

class CollectDeclRef : public MatchFinder::MatchCallback {
public:
  std::set<const clang::Expr *> d_refs;
  CollectDeclRef() : d_refs() {}

  virtual void run(const MatchFinder::MatchResult &result) override {
    if (const auto *ref = result.Nodes.getNodeAs<clang::Expr>("context")) {
      d_refs.insert(ref);
    }
  }
};

class TestTool {
public:
  typedef std::string ArgTypes;

private:
  clang::CompilerInstance *d_ci;
  StatementMatcher d_globalVariableRef;
  CollectDeclRef d_callback;
  ArgTypes d_parentFuncDeclName;

public:
  TestTool(clang::CompilerInstance *ci, MatchFinder *f, ArgTypes parentName)
      : d_ci(ci),
        d_globalVariableRef(globalArrayRefWithName("_GLOBALARRAY", parentName)),
        d_callback(), d_parentFuncDeclName(parentName) {
    f->addMatcher(d_globalVariableRef, &d_callback);
  }

  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    // These were all the refs to the variable we declared
    for (auto expr : d_callback.d_refs) {
      if (d_parentFuncDeclName == "positive") {
        ASSERT_TRUE(clangmetatool::SourceUtil::isPartialMacro(
            expr->getSourceRange(), d_ci->getSourceManager(),
            d_ci->getPreprocessor()));
      } else if (d_parentFuncDeclName == "negative") {
        ASSERT_FALSE(clangmetatool::SourceUtil::isPartialMacro(
            expr->getSourceRange(), d_ci->getSourceManager(),
            d_ci->getPreprocessor()));
      } else {
        llvm::errs() << "Invalid branch\n";
        std::terminate();
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
      "/t/data/033-detect-partial-macro-expansion/partial-macro-usages.cpp",
      "--", "-xc++"};

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, category, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  std::string positive("positive");
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<TestTool>> raf(
      tool.getReplacements(), positive);
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
      "--", "-xc++"};

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, category, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  std::string negative("negative");
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<TestTool>> raf(
      tool.getReplacements(), negative);
  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}

TEST(partialMacroExpansion_Detect, DISABLED_FalseNegatives) {
  llvm::cl::OptionCategory category("my-tool options");
  int argc = 4;
  const char *argv[] = {
      "foo",
      CMAKE_SOURCE_DIR
      "/t/data/033-detect-partial-macro-expansion/partial-macro-usages.cpp",
      "--", "-xc++"};

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, category, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  std::string negative("falseNegative");
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<TestTool>> raf(
      tool.getReplacements(), negative);
  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
}
