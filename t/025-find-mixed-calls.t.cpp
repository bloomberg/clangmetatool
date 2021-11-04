#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/find_calls.h>
#include <clangmetatool/collectors/find_cxx_member_calls.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

class MyTool {
private:
  clang::CompilerInstance* ci;

  std::string func = std::string("foo");

  std::string cNC = std::string("NakedClass");
  std::string nNCfoo = std::string("foo");
  clangmetatool::collectors::FindCXXMemberCalls fcNCfoo;
  std::string nNCbar = std::string("bar");
  clangmetatool::collectors::FindCXXMemberCalls fcNCbar;

  std::string cBC = std::string("suit::BusinessClass");
  std::string nBCbook = std::string("book");
  clangmetatool::collectors::FindCXXMemberCalls fcBCbook;
  std::string nBCdiatribe = std::string("diatribe");
  clangmetatool::collectors::FindCXXMemberCalls fcBCdiatribe;

  std::string nHurdur = std::string("hurdur");
  clangmetatool::collectors::FindCalls fcHurdur;
public:

  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    : ci(ci)
    , fcNCfoo(ci, f, cNC, nNCfoo)
    , fcNCbar(ci, f, cNC, nNCbar)
    , fcBCbook(ci, f, cBC, nBCbook)
    , fcBCdiatribe(ci, f, cBC, nBCdiatribe)
    , fcHurdur(ci, f, nHurdur)
  {}

  void validateMemberFind(clangmetatool::collectors::FindCXXMemberCalls& fc,
                          const std::string& c, const std::string n) {
    const auto data = fc.getData();

    ASSERT_EQ(1, data->size());

    auto found = data->begin();

    EXPECT_EQ(func, found->first->getNameInfo().getAsString());
    EXPECT_EQ(c, found->second->getRecordDecl()->getQualifiedNameAsString());
    EXPECT_EQ(n, found->second->getMethodDecl()->getNameAsString());
  }

  void postProcessing
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    validateMemberFind(fcNCfoo, cNC, nNCfoo);
    validateMemberFind(fcNCbar, cNC, nNCbar);
    validateMemberFind(fcBCbook, cBC, nBCbook);
    validateMemberFind(fcBCdiatribe, cBC, nBCdiatribe);

    auto data = fcHurdur.getData();
    ASSERT_EQ(1, data->call_context.size());
    const auto& item = *data->call_context.begin();
    EXPECT_EQ(func, item.first->getNameAsString());
    EXPECT_EQ(nHurdur, item.second->getDirectCallee()->getNameAsString());
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    CMAKE_SOURCE_DIR "/t/data/025-find-mixed-calls/foo.cpp",
    "--",
    "-xc++"
  };

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();

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
