#include "clangmetatool-testconfig.h"

#include <memory>
#include <string>
#include <unordered_set>

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/find_functions.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

namespace {

class MyTool {
public:
  using Container = std::unordered_set<std::string>;
  using ArgTypes = std::shared_ptr<Container>;

private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::FindFunctions ff;
  ArgTypes fns;

public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f,
         ArgTypes fns)
    :ci(ci), ff(ci, f), fns(fns) {}

  void postProcessing
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    for (const auto& fn : *ff.getData()) {
      fns->insert(fn->getQualifiedNameAsString());
    }
  }
};

void runTool(const std::string& file, MyTool::ArgTypes& fns) {
  std::string fullPath = CMAKE_SOURCE_DIR "/t/data/032-find-functions/" + file;
  
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "foo",
    fullPath.c_str(),
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
    raf(tool.getReplacements(), fns);

  int r = tool.run(&raf);
  ASSERT_EQ(0, r);
}

} // anonymous namespace

TEST(FindFunctions, functions) {
  MyTool::ArgTypes fns = std::make_shared<MyTool::Container>();

  runTool("functions.cpp", fns);

  ASSERT_EQ(7, fns->size());
  EXPECT_TRUE(fns->end() != fns->find("foo2"));
  EXPECT_TRUE(fns->end() != fns->find("foo3"));
  EXPECT_TRUE(fns->end() != fns->find("foo::bar"));
  EXPECT_TRUE(fns->end() != fns->find("foobar"));
  EXPECT_TRUE(fns->end() != fns->find("mwahaha"));
  EXPECT_TRUE(fns->end() != fns->find("yolo::bar"));
  EXPECT_TRUE(fns->end() != fns->find("yolo_ono"));
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
