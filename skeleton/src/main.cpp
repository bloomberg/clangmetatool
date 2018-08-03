#include <iosfwd>
#include <map>
#include <tuple>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>

class MyTool {
public:
  typedef std::tuple<> ArgTypes;
  MyTool(clang::CompilerInstance* ci,
         clang::ast_matchers::MatchFinder *f,
         ArgTypes arguments) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
  }
};

int main(int argc, const char* argv[]) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  llvm::cl::extrahelp CommonHelp
    (clang::tooling::CommonOptionsParser::HelpMessage);

  clang::tooling::CommonOptionsParser
    optionsParser(argc, argv, MyToolCategory);

  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());

  MyTool::ArgTypes toolArgs;
  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements(), toolArgs);

  int r = tool.runAndSave(&raf);

  return r;
}

