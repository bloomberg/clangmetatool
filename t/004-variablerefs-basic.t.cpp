#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/variable_refs_data.h>
#include <clangmetatool/collectors/variable_refs.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/ADT/APSInt.h>

class MyTool {
public:
  typedef std::tuple<> ArgTypes;
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::VariableRefs v;
  ArgTypes args;

public:
  MyTool(clang::CompilerInstance* ci,
         clang::ast_matchers::MatchFinder *f,
         ArgTypes args)
    :ci(ci), v(ci, f), args(args) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    const clang::ASTContext &ctx = ci->getASTContext();

    std::multimap<
      const clang::VarDecl*,
      const clang::DeclRefExpr*
      > *refs = &(v.getData()->refs);

    ASSERT_EQ(6, refs->size())
      << "Found decl ref";

    // these are the values and names from the references in the order
    // that we see them in the code.
    int64_t values[] = { 3, 2, 5, 5, 6, 11 };
    const char* names[] = { "a", "b", "c", "d", "e", "f" };

    int count = 0;
    auto it = refs->begin();
    while (it != refs->end()) {
      const clang::VarDecl* var = it->first;
      const clang::DeclRefExpr* ref = it->second;


      ASSERT_EQ(std::string(names[count]), var->getNameAsString())
        << "Got the expected variable name " << count;

      llvm::APSInt value;
      bool eval = ref->EvaluateAsInt
        ( value, ctx,
          clang::Expr::SideEffectsKind::SE_AllowSideEffects
          );

      ASSERT_EQ(true, eval)
        << "Value can be evaluated " << names[count] << " " << count;

      ASSERT_EQ(values[count], value.getExtValue())
        << "Has the expected value " << names[count] << " " << count;

      count++;
      it++;
    }

  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/004-variablerefs-basic/foo.cpp",
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

  MyTool::ArgTypes toolArgs;
  clangmetatool::MetaToolFactory< clangmetatool::MetaTool<MyTool> >
    raf(tool.getReplacements(), toolArgs);

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
