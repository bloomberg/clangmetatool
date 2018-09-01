#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/meta_tool_factory.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/collectors/files.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/Refactoring.h>
#include <llvm/Support/CommandLine.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::Files dc;
public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), dc(ci, f) {
  }
  void postProcessing
  (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    clangmetatool::collectors::FilesData *d = dc.getData();

    std::vector<std::string> file_names_expected({
        "bar.cpp",
        "foo.cpp"
    });
    std::vector<std::string> file_names_actual;

    size_t num_files_expected = file_names_expected.size();

    ASSERT_EQ(num_files_expected, d->filePath.size())
      << "Has the right number of files";

    for (auto const& file_pair : d->filePath) {
        std::string fullpath = file_pair.second;

        // Extract filename from path
        boost::filesystem::path p(fullpath);
        boost::filesystem::path fn = p.filename();
        std::string filename = fn.string();

        file_names_actual.push_back(filename);
    }

    std::sort(file_names_actual.begin(), file_names_actual.end());

    for (size_t i = 0; i < num_files_expected; ++i) {
        ASSERT_EQ(file_names_expected[i], file_names_actual[i])
            << "File names match";
    }
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char* argv[] = {
    "files",
    CMAKE_SOURCE_DIR "/t/data/019-files/foo.cpp",
    //CMAKE_SOURCE_DIR "/t/data/019-files/bar.cpp",
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
