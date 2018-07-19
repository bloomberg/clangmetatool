#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/defs.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

#include <iostream>
#include <vector>
#include <algorithm>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::DefCollector dc;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), dc(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    clangmetatool::collectors::DefData *d = dc.getData();

    std::vector<std::string> func_names_expected({
        "bar",
        "foo"
        });
    std::vector<std::string> func_names_actual;

    size_t num_funcs_expected = func_names_expected.size();

    ASSERT_EQ(num_funcs_expected, d->defs.size())
      << "Has the right number of functions";

    for (auto const& def_pair : d->defs) {
        func_names_actual.push_back(def_pair.second->getNameAsString());
    }

    std::sort(func_names_actual.begin(), func_names_actual.end());

    for (size_t i = 0; i < num_funcs_expected; ++i) {
        ASSERT_EQ(func_names_expected[i], func_names_actual[i])
            << "Function name matches";
    }
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "two-funcs",
    CMAKE_SOURCE_DIR "/t/data/007-defs-two-funcs/two-funcs.cpp",
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
