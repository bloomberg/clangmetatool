#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

#include <tuple>

bool constructor_called;
bool postprocessing_called;

class MyTool {
public:
  typedef std::tuple<char, int, std::string> ArgTypes;

private:
  clang::CompilerInstance* ci;
  clang::ast_matchers::MatchFinder *f;
  ArgTypes args;

public:
  MyTool(clang::CompilerInstance* ci,
         clang::ast_matchers::MatchFinder *f,
         ArgTypes& args)
    :ci(ci), f(f), args(args) {
    constructor_called = true;
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    ASSERT_NE((void*)NULL, ci);
    ASSERT_NE((void*)NULL, f);

    EXPECT_EQ('$',                          std::get<0>(args));
    EXPECT_EQ(42,                           std::get<1>(args));
    EXPECT_EQ(std::string("the answer"),    std::get<2>(args));

    postprocessing_called = true;
  }
};

// A struct to pass as an argument to MyTool
struct AStruct {
    int dummyField;
    AStruct() : dummyField(0) {};
    bool operator==(const AStruct& other) const {
        return dummyField == other.dummyField;
    }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = { "foo", "/dev/null", "--", "-xc++"  };

  auto result = clang::tooling::CommonOptionsParser::create(
    argc, argv, MyToolCategory, llvm::cl::OneOrMore);
  ASSERT_TRUE(!!result);
  clang::tooling::CommonOptionsParser& optionsParser = result.get();

  clang::tooling::RefactoringTool tool
    ( optionsParser.getCompilations(),
      optionsParser.getSourcePathList());

  constructor_called = false;
  postprocessing_called = false;

  MyTool::ArgTypes toolArgs('$', 42, "the answer");
  clangmetatool::MetaToolFactory<
      clangmetatool::MetaTool< MyTool > > raf(tool.getReplacements(), toolArgs);

  int r = tool.runAndSave(&raf);
  ASSERT_EQ(0, r);
  ASSERT_EQ(true, constructor_called);
  ASSERT_EQ(true, postprocessing_called);

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
