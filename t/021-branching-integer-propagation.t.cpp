#include "clangmetatool-testconfig.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/propagation/constant_integer_propagator.h>
#include <llvm/Support/CommandLine.h>

#include <gtest/gtest.h>

namespace {

using namespace clang::ast_matchers;

using FindVarDeclsDatum =
    std::pair<const clang::FunctionDecl *, const clang::DeclRefExpr *>;
using FindVarDeclsData = std::vector<FindVarDeclsDatum>;

class FindVarDeclsCallback : public MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  FindVarDeclsData *data;

public:
  FindVarDeclsCallback(clang::CompilerInstance *ci, FindVarDeclsData *data)
      : ci(ci), data(data) {}

  virtual void run(const MatchFinder::MatchResult &r) override {
    const clang::FunctionDecl *f =
        r.Nodes.getNodeAs<clang::FunctionDecl>("func");

    const clang::DeclRefExpr *d =
        r.Nodes.getNodeAs<clang::DeclRefExpr>("declRef");

    data->push_back({f, d});
  }
};

class FindVarDecls {
private:
  FindVarDeclsData data;

  StatementMatcher matcher =
      (declRefExpr(hasDeclaration(varDecl(hasAnyName("v1"))),
                   hasAncestor(functionDecl().bind("func"))))
          .bind("declRef");

  FindVarDeclsCallback callback;

public:
  FindVarDecls(clang::CompilerInstance *ci, MatchFinder *f)
      : callback(ci, &data) {
    f->addMatcher(matcher, &callback);
  }

  FindVarDeclsData *getData() { return &data; }
};

class MyTool {
private:
  clang::CompilerInstance *ci;
  FindVarDecls fvd;
  clangmetatool::propagation::ConstantIntegerPropagator cip;

public:
  MyTool(clang::CompilerInstance *ci, MatchFinder *f)
      : ci(ci), fvd(ci, f), cip(ci) {}

  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    FindVarDeclsData *decls = fvd.getData();

    for (auto decl : *decls) {
      cip.runPropagation(decl.first, decl.second);
    }

    std::ostringstream stream;

    cip.dump(stream);

    const char *expectedResult =
        "main >>>>>>>>>>>>>>>>>>>>>>>>>>\n"
        "  ** v1\n"
        "    - 5:3 '1' (Changed by code)\n"
        "    - 16:3 '0' (Changed by code)\n"
        "  ** v2\n"
        "    - 6:3 '2' (Changed by code)\n"
        "    - 11:5 '5' (Changed by code)\n"
        "    - 13:5 '2' (Control flow merge)\n"
        "    - 16:3 '<UNRESOLVED>' (Control flow merge)\n"
        "  ** v3\n"
        "    - 7:3 '3' (Changed by code)\n"
        "    - 13:5 '6' (Changed by code)\n"
        "    - 16:3 '<UNRESOLVED>' (Control flow merge)\n"
        "  ** v4\n"
        "    - 8:3 '4' (Changed by code)\n"
        "main <<<<<<<<<<<<<<<<<<<<<<<<<<\n";

    EXPECT_STREQ(expectedResult, stream.str().c_str());
  }
};

} // namespace anonymous

TEST(propagation_ConstantIntegerPropagation, functionArgs) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char *argv[] = {"foo", CMAKE_SOURCE_DIR
                        "/t/data/021-branching-integer-propagation/main.c",
                        "--",
                        // Test C++ for confirming that this works on a superset
                        "-xc"};
  clang::tooling::CommonOptionsParser optionsParser(argc, argv, MyToolCategory);
  clang::tooling::RefactoringTool tool(optionsParser.getCompilations(),
                                       optionsParser.getSourcePathList());
  clangmetatool::MetaToolFactory<clangmetatool::MetaTool<MyTool>> raf(
      tool.getReplacements());
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
