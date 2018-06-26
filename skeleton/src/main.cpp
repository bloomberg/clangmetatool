#include <iosfwd>
#include <map>

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
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f) {
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

  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements());

  int r = tool.runAndSave(&raf);

  return r;
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
