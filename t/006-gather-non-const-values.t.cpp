#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/collectors/variable_refs.h>
#include <clangmetatool/collectors/variable_refs_data.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance *ci;
  clangmetatool::collectors::VariableRefs v;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), v(ci, f) {}
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    const clang::ASTContext &ctx = ci->getASTContext();
    clangmetatool::collectors::VariableRefsData *data = v.getData();

    auto it = data->refs.begin();
    ASSERT_NE(data->refs.end(), it);

    const clang::VarDecl *var = it->first;
    const clang::DeclRefExpr *ref = it->second;

    ASSERT_EQ(std::string("a"), var->getNameAsString())
        << "Got the expected variable name for a";

    const clang::Expr *init = var->getAnyInitializer();
    ASSERT_NE((void *)NULL, init) << "Found the initializer for a";

    llvm::APSInt value;
    bool eval = init->EvaluateAsInt(
        value, ctx, clang::Expr::SideEffectsKind::SE_AllowSideEffects);

    ASSERT_EQ(true, eval) << "Initalizer for a can be evaluated";

    ASSERT_EQ(3, value.getExtValue())
        << "Iniitalizer for a has the right value";

    ASSERT_EQ(0, data->clear_assignment_refs.count(ref))
        << "The ref to a is not an assignment";

    ASSERT_EQ(1, data->rvalue_refs.count(ref)) << "The ref to a is an rvalue";

    it++;
    var = it->first;
    ref = it->second;

    ASSERT_NE(data->refs.end(), it);

    ASSERT_EQ(std::string("b"), var->getNameAsString())
        << "Got the expected variable name for b";

    init = var->getAnyInitializer();
    ASSERT_EQ((void *)NULL, init) << "b has no initializer";

    ASSERT_EQ(0, data->rvalue_refs.count(ref))
        << "The first ref to b is not an rvalue";

    ASSERT_EQ(1, data->clear_assignment_refs.count(ref))
        << "The first ref to b is an assignment";

    init = data->clear_assignment_refs[ref];
    ASSERT_NE((void *)NULL, init) << "Found the expression for the assignment";

    eval = init->EvaluateAsInt(
        value, ctx, clang::Expr::SideEffectsKind::SE_AllowSideEffects);

    ASSERT_EQ(true, eval) << "Expression used in b assignment can be evaluated";

    ASSERT_EQ(2, value.getExtValue()) << "Assignment to b has the right value";

    it++;
    var = it->first;
    ref = it->second;

    ASSERT_NE(data->refs.end(), it);

    ASSERT_EQ(std::string("b"), var->getNameAsString())
        << "Got the second ref to b";

    ASSERT_EQ(1, data->rvalue_refs.count(ref))
        << "The second ref to b is an rvalue";

    ASSERT_EQ(0, data->clear_assignment_refs.count(ref))
        << "The second ref to b is not an assignment";

    it++;

    ASSERT_EQ(data->refs.end(), it) << "Found all refs";
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {"foo", CMAKE_SOURCE_DIR
                        "/t/data/006-gather-non-const-values/foo.cpp",
                        "--", "-xc++"};

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
