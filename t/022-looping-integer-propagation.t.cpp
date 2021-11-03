#include "clangmetatool-testconfig.h"

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
#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/propagation/constant_integer_propagator.h>

#include <gtest/gtest.h>

namespace {

using namespace clang::ast_matchers;

using FindVarDeclsDatum = std::pair<const clang::FunctionDecl*, const clang::DeclRefExpr*>;
using FindVarDeclsData  = std::vector<FindVarDeclsDatum>;

class FindVarDeclsCallback : public MatchFinder::MatchCallback {
private:
  clang::CompilerInstance* ci;
  FindVarDeclsData* data;

public:
  FindVarDeclsCallback(clang::CompilerInstance* ci, FindVarDeclsData* data)
    : ci(ci), data(data) {
  }

  virtual void run(const MatchFinder::MatchResult& r) override {
    const clang::FunctionDecl* f = r.Nodes.getNodeAs<clang::FunctionDecl>("func");

    const clang::DeclRefExpr* d = r.Nodes.getNodeAs<clang::DeclRefExpr>("declRef");

    data->push_back({f, d});
  }
};

class FindVarDecls {
private:
  FindVarDeclsData data;

  StatementMatcher matcher =
    (declRefExpr
     (hasDeclaration
      (varDecl
       (hasAnyName
        (
         "v1"
        )
       )
      ),
      hasAncestor(
       functionDecl().bind("func")
      )
     )
    ).bind("declRef");

  FindVarDeclsCallback callback;

public:
  FindVarDecls(clang::CompilerInstance* ci,
               MatchFinder*             f)
    : callback(ci, &data)
  {
    f->addMatcher(matcher, &callback);
  }

  FindVarDeclsData* getData() { return &data; }
};

class MyTool {
private:
  clang::CompilerInstance* ci;
  FindVarDecls fvd;
  clangmetatool::propagation::ConstantIntegerPropagator cip;

public:
  MyTool(clang::CompilerInstance* ci, MatchFinder *f)
    : ci(ci), fvd(ci, f), cip(ci) {
  }

  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    FindVarDeclsData* decls = fvd.getData();

    for(auto decl : *decls) {
      cip.runPropagation(decl.first, decl.second);
    }

    std::ostringstream stream;

    cip.dump(stream);

    const char* expectedResult =
      "main >>>>>>>>>>>>>>>>>>>>>>>>>>\n"
      "  ** i\n"
      "    - 14:7 '0' (Changed by code)\n"
      "  ** v1\n"
      "    - 5:3 '1' (Changed by code)\n"
      "    - 18:3 '0' (Changed by code)\n"
      "  ** v2\n"
      "    - 6:3 '2' (Changed by code)\n"
      "    - 11:5 '5' (Changed by code)\n"
      "    - 14:7 '<UNRESOLVED>' (Control flow merge)\n"
      "  ** v3\n"
      "    - 7:3 '3' (Changed by code)\n"
      "    - 15:5 '6' (Changed by code)\n"
      "    - 18:3 '<UNRESOLVED>' (Control flow merge)\n"
      "  ** v4\n"
      "    - 8:3 '4' (Changed by code)\n"
      "    - 21:5 '7' (Changed by code)\n"
      "main <<<<<<<<<<<<<<<<<<<<<<<<<<\n";

    EXPECT_STREQ(expectedResult, stream.str().c_str());
  }
};

} // namespace anonymous

TEST(propagation_ConstantIntegerPropagation, functionArgs) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/022-looping-integer-propagation/main.c",
    "--",
    // Test C++ for confirming that this works on a superset
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
