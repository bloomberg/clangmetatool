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
#include <clang/Basic/SourceManager.h>


#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/propagation/constant_cstring_propagator.h>
#include <clangmetatool/collectors/find_calls.h>

#include <gtest/gtest.h>

namespace {

using namespace clang::ast_matchers;

using FindCalls = clangmetatool::collectors::FindCalls;

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
  clangmetatool::propagation::ConstantCStringPropagator csp;

  std::unique_ptr<FindCalls> callObject;



public:
  MyTool(clang::CompilerInstance* ci, MatchFinder *f)
    : ci(ci), fvd(ci, f), csp(ci) {
        callObject = std::unique_ptr<FindCalls>(new FindCalls(ci, f, "foo")); // initialize function finder
  }


  bool compSrcLoc(const clang::SourceLocation& LHS,
                const clang::SourceLocation& RHS,
                clang::SourceManager& SM){
      clang::BeforeThanCompare<clang::SourceLocation> btc(SM);
      return btc(LHS, RHS);
  }

  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    FindVarDeclsData* decls = fvd.getData();

    

    EXPECT_EQ(decls->size(), 2);

    auto macroStr = decls->at(0).second->getLocStart().printToString(ci->getSourceManager()); 
    EXPECT_EQ("5:3", macroStr.substr(macroStr.find(":", 0) + 1, 3));
    // FUNC(v1)
    
    auto funcStr = decls->at(1).second->getLocStart().printToString(ci->getSourceManager()); 
    EXPECT_EQ("6:7", funcStr.substr(macroStr.find(":", 0) + 1, 3));
    // foo(v1);

    EXPECT_FALSE(decls->at(0).second->getLocStart() < decls->at(1).second->getLocStart());
    // Using "<" for comparing source location is not correct 

    EXPECT_TRUE(compSrcLoc(decls->at(0).second->getLocStart(),
                           decls->at(1).second->getLocStart(),
                           ci->getASTContext().getSourceManager()));


    auto call_context = callObject->getData()->call_context;
    const clang::FunctionDecl *decl;
    const clang::CallExpr *call;

    EXPECT_EQ(2, call_context.size());
    const auto &p = call_context.begin(); // Check only the first function call

    std::tie(decl, call) = (*p);
    const auto result = csp.runPropagation(decl, reinterpret_cast<const clang::DeclRefExpr *> (call->getArg(0)->IgnoreImplicit()));

    std::ostringstream temp;
    result.print(temp);
    EXPECT_STREQ("something", temp.str().c_str());
    
    
    
  }
};

} // namespace anonymous

TEST(propagation_MacroConstantPropagation, basic) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/026-macro-propagation/foo.cpp",
    "--",
    "-xc"
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
