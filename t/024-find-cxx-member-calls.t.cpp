#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/collectors/find_cxx_member_calls.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance *ci;

  std::string cNS = std::string("NakedStruct");
  std::string nNSfoo = std::string("foo");
  clangmetatool::collectors::FindCXXMemberCalls fcNSfoo;
  std::string nNSbar = std::string("bar");
  clangmetatool::collectors::FindCXXMemberCalls fcNSbar;

  std::string cNC = std::string("NakedClass");
  std::string nNCfoo = std::string("foo");
  clangmetatool::collectors::FindCXXMemberCalls fcNCfoo;
  std::string nNCbar = std::string("bar");
  clangmetatool::collectors::FindCXXMemberCalls fcNCbar;

  std::string cBS = std::string("suit::BusinessStruct");
  std::string nBSfoo = std::string("foo");
  clangmetatool::collectors::FindCXXMemberCalls fcBSfoo;
  std::string nBSbar = std::string("bar");
  clangmetatool::collectors::FindCXXMemberCalls fcBSbar;

  std::string cBC = std::string("suit::BusinessClass");
  std::string nBCbook = std::string("book");
  clangmetatool::collectors::FindCXXMemberCalls fcBCbook;
  std::string nBCdiatribe = std::string("diatribe");
  clangmetatool::collectors::FindCXXMemberCalls fcBCdiatribe;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), fcNSfoo(ci, f, cNS, nNSfoo), fcNSbar(ci, f, cNS, nNSbar),
        fcNCfoo(ci, f, cNC, nNCfoo), fcNCbar(ci, f, cNC, nNCbar),
        fcBSfoo(ci, f, cBS, nBSfoo), fcBSbar(ci, f, cBS, nBSbar),
        fcBCbook(ci, f, cBC, nBCbook), fcBCdiatribe(ci, f, cBC, nBCdiatribe) {}

  static void validateFind(clangmetatool::collectors::FindCXXMemberCalls &fc,
                           const std::string &c, const std::string n) {
    const auto data = fc.getData();

    ASSERT_EQ(1, data->size());

    auto found = data->begin();

    EXPECT_EQ("foo", found->first->getNameInfo().getAsString());
    EXPECT_EQ(c, found->second->getRecordDecl()->getQualifiedNameAsString());
    EXPECT_EQ(n, found->second->getMethodDecl()->getNameAsString());
  }

  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    validateFind(fcNSfoo, cNS, nNSfoo);
    validateFind(fcNSbar, cNS, nNSbar);
    validateFind(fcNCfoo, cNC, nNCfoo);
    validateFind(fcNCbar, cNC, nNCbar);
    validateFind(fcBSfoo, cBS, nBSfoo);
    validateFind(fcBSbar, cBS, nBSbar);
    validateFind(fcBCbook, cBC, nBCbook);
    validateFind(fcBCdiatribe, cBC, nBCdiatribe);
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {"foo", CMAKE_SOURCE_DIR
                        "/t/data/024-find-cxx-member-calls/foo.cpp",
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
