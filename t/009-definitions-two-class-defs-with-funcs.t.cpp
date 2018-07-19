#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/definitions.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

#include <iostream>
#include <typeinfo>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::Definitions dc;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), dc(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    clangmetatool::collectors::DefinitionsData *d = dc.getData();

#if 1
    for (auto const& def_pair : d->defs) {
        std::cerr << def_pair.second->getNameAsString() << std::endl;
    }
#endif

    std::vector<std::string> def_names_expected({
            "Bar",
            "Bar",
            "Foo",
            "initialized_global_var",
            "member_func_of_foo",
            "uninitialized_global_var"
        });
    std::vector<std::string> def_names_actual;

    size_t num_funcs_expected = def_names_expected.size();

    ASSERT_EQ(num_funcs_expected, d->defs.size())
      << "Has the right number of functions";

    for (auto const& def_pair : d->defs) {
        def_names_actual.push_back(def_pair.second->getNameAsString());
    }

    std::sort(def_names_actual.begin(), def_names_actual.end());

    for (size_t i = 0; i < num_funcs_expected; ++i) {
        ASSERT_EQ(def_names_expected[i], def_names_actual[i])
            << "Function name matches";
    }
#if 0
    EXPECT_TRUE(false) << "Force failure to see output of test";
#endif
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "two-class-defs-with-funcs",
    CMAKE_SOURCE_DIR "/t/data/009-definitions-two-class-defs-with-funcs/two-class-defs-with-funcs.cpp",
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
