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
#include <utility>

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

#if 0
    for (auto const& ref_pair : d->refs) {
        std::cerr << typeid(*ref_pair.first).name() << " -> ";
        std::cerr << typeid(*ref_pair.second).name() << std::endl;
        std::cerr << ref_pair.first->getNameAsString() << " -> ";
        std::cerr << ref_pair.second->getNameAsString() << std::endl;
        std::cerr << std::endl;
    }
#endif

    std::vector<std::pair<std::string, std::string> > ref_names_expected({
            std::make_pair("Foo", "Foo"),
            std::make_pair("Foo", "Foo"),
            std::make_pair("Foo", "func2"),
            std::make_pair("Foo", "global_foo"),
            std::make_pair("bar_func", "Bar"),
            std::make_pair("called_func", "func"),
            std::make_pair("called_func", "func"),
            std::make_pair("called_func_2", "global_uncalled_func"),
            std::make_pair("called_func_2", "global_var_3"),
            std::make_pair("extern_var", "func"),
            std::make_pair("f", "Bar"),
            std::make_pair("foo_mem", "Foo"),
            std::make_pair("foo_mem", "Foo"),
            std::make_pair("foo_mem", "Foo"),
            std::make_pair("global_var", "func"),
            std::make_pair("global_var", "func"),
            std::make_pair("global_var", "global_var_2"),
            std::make_pair("global_var", "global_var_3"),
            std::make_pair("global_var", "static_local_var")
        });
    std::vector<std::pair<std::string, std::string> > ref_names_actual;

    size_t num_refs_expected = ref_names_expected.size();

    ASSERT_EQ(num_refs_expected, d->refs.size())
        << "Has the right number of references";

    for (auto const& ref_pair : d->refs) {
        ref_names_actual.push_back(
                std::make_pair(
                    ref_pair.first->getNameAsString(),
                    ref_pair.second->getNameAsString()
                    )
                );
    }

    std::sort(ref_names_actual.begin(), ref_names_actual.end());

    for (size_t i = 0; i < num_refs_expected; ++i) {
        ASSERT_EQ(ref_names_expected[i], ref_names_actual[i])
            << "Reference names match";
    }

#if 0
    for (auto const& dep_pair : d->deps) {
        std::cerr << typeid(*dep_pair.first).name() << " -> ";
        std::cerr << typeid(*dep_pair.second).name() << std::endl;
        std::cerr << dep_pair.first->getNameAsString() << " -> ";
        std::cerr << dep_pair.second->getNameAsString() << std::endl;
        std::cerr << std::endl;
    }
#endif

    std::vector<std::pair<std::string, std::string> > dep_names_expected({
            std::make_pair("Bar", "bar_func"),
            std::make_pair("Bar", "f"),
            std::make_pair("Foo", "Foo"),
            std::make_pair("Foo", "Foo"),
            std::make_pair("Foo", "foo_mem"),
            std::make_pair("Foo", "foo_mem"),
            std::make_pair("Foo", "foo_mem"),
            std::make_pair("func", "called_func"),
            std::make_pair("func", "called_func"),
            std::make_pair("func", "extern_var"),
            std::make_pair("func", "global_var"),
            std::make_pair("func", "global_var"),
            std::make_pair("func2", "Foo"),
            std::make_pair("global_foo", "Foo"),
            std::make_pair("global_uncalled_func", "called_func_2"),
            std::make_pair("global_var_2", "global_var"),
            std::make_pair("global_var_3", "called_func_2"),
            std::make_pair("global_var_3", "global_var"),
            std::make_pair("static_local_var", "global_var")
        });
    std::vector<std::pair<std::string, std::string> > dep_names_actual;

    size_t num_deps_expected = dep_names_expected.size();

    ASSERT_EQ(num_deps_expected, d->deps.size())
        << "Has the right number of dependencies";

    for (auto const& dep_pair : d->deps) {
        dep_names_actual.push_back(
                std::make_pair(
                    dep_pair.first->getNameAsString(),
                    dep_pair.second->getNameAsString()
                    )
                );
    }

    std::sort(dep_names_actual.begin(), dep_names_actual.end());

    for (size_t i = 0; i < num_deps_expected; ++i) {
        ASSERT_EQ(dep_names_expected[i], dep_names_actual[i])
            << "Dependency names match";
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
    "references-template",
    CMAKE_SOURCE_DIR "/t/data/013-references-template/template.cpp",
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
