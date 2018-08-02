#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/find_calls.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

#include <tuple>

template <class... Args>
class MyTool {
private:
  std::string n = std::string("bar");
  clang::CompilerInstance* ci;
  clangmetatool::collectors::FindCalls fc;
  std::tuple<Args...> additionalArgs;

public:

  MyTool(clang::CompilerInstance* ci,
         clang::ast_matchers::MatchFinder *f,
         std::tuple<Args...> args)
    :ci(ci), fc(ci, f, n), additionalArgs(args) {}

  const clang::DeclRefExpr* get_pointer_to(const clang::Expr* expr) {
    const clang::DeclRefExpr* ret = NULL;
    const clang::UnaryOperator* op =
      llvm::dyn_cast_or_null<const clang::UnaryOperator>(expr);
    if (op != NULL) {
      if (op->getOpcode() == clang::UnaryOperatorKind::UO_AddrOf) {
        ret = llvm::dyn_cast_or_null<const clang::DeclRefExpr>
          (op->getSubExpr());
      }
    }
    return ret;
  }

  void postProcessing
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
      clang::SourceManager& sm = ci->getSourceManager();

      auto call_context = &(fc.getData()->call_context);
      auto ccit = call_context->begin();

      auto call_ref = &(fc.getData()->call_ref);
      auto rcit = call_ref->begin();

      const clang::FunctionDecl *caller = ccit->first;
      const clang::CallExpr *callee = ccit->second;
      const clang::FunctionDecl* called = callee->getDirectCallee();
      ASSERT_EQ(std::string("foo"), caller->getNameAsString());
      ASSERT_EQ(std::string("bar"), called->getNameAsString());

      }
  };

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/007-find-calls/foo.cpp",
    "--",
    "-xc++"
  };

  clang::tooling::CommonOptionsParser
    optionsParser
    ( argc, argv,
      MyToolCategory );
  clang::tooling::RefactoringTool tool
    ( optionsParser.getCompilations(),
      optionsParser.getSourcePathList());

  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
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
