#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/references.h>

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
  clangmetatool::collectors::References dc;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), dc(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    clangmetatool::collectors::ReferencesData *d = dc.getData();

#if 1
    for (auto const& ref_pair : d->refs) {
        std::cerr << typeid(*ref_pair.second).name() << ": ";
        //std::cerr << ref_pair.second->getNameAsString() << std::endl;
    }
#endif

    std::vector<std::string> ref_names_expected({
        });
    std::vector<std::string> ref_names_actual;

    size_t num_funcs_expected = ref_names_expected.size();

    ASSERT_EQ(num_funcs_expected, d->refs.size())
      << "Has the right number of references";

    for (auto const& ref_pair : d->refs) {
        // ref_names_actual.push_back(ref_pair.second->getNameAsString());
    }

    std::sort(ref_names_actual.begin(), ref_names_actual.end());

    for (size_t i = 0; i < num_funcs_expected; ++i) {
        ASSERT_EQ(ref_names_expected[i], ref_names_actual[i])
            << "Reference name matches";
    }
#if 1
    EXPECT_TRUE(false) << "Force failure to see output of test";
#endif
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "references-non-template",
    CMAKE_SOURCE_DIR "/t/data/012-references-non-template.cpp",
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
