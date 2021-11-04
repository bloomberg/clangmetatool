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
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/YAMLTraits.h>

namespace {

class MyTool {
private:
  clang::CompilerInstance* ci;
  clangmetatool::collectors::FindFunctions ff;

public:
  MyTool(clang::CompilerInstance* ci, clang::ast_matchers::MatchFinder *f)
    :ci(ci), ff(ci, f) {}

  void postProcessing
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap) {
    for (const auto& fn : *ff.getData()) {
      auto nameSource = fn->getNameInfo().getSourceRange();
      clang::tooling::Replacement r(ci->getSourceManager(), nameSource.getBegin(),
                                    0, "new_");
      auto filename = ci->getSourceManager().getFilename(nameSource.getBegin());
      auto err = replacementsMap[filename.str()].add(r);
    }
  }
};

static void eatDiagnostics(const llvm::SMDiagnostic &, void *) {}

} // anonymous namespace

TEST(ValidateOutput, sourceAndHeaderFile) {
  std::string srcPath = CMAKE_SOURCE_DIR "/t/data/036-export-fixes-output/functions.cpp";
  std::string headerPath = CMAKE_SOURCE_DIR "/t/data/036-export-fixes-output/include.h";

  std::set<clang::tooling::Replacement> expectedReplacements;
  auto it = expectedReplacements.emplace(clang::tooling::Replacement(srcPath, 26, 0, "new_"));
  it = expectedReplacements.emplace(clang::tooling::Replacement(headerPath, 5, 0, "new_"));

  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 5;
  const char* argv[] = {
    "foo",
    srcPath.c_str(),
    headerPath.c_str(),
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

  // ReplList repls;

  std::string O;
  llvm::raw_string_ostream os(O);
  int r = raf.runAndExportFixes(tool, os);
  llvm::yaml::Input replacementsYaml(os.str(), nullptr, &eatDiagnostics);
  clang::tooling::TranslationUnitReplacements TURs;
  replacementsYaml >> TURs;
  ASSERT_EQ(0, r);
  ASSERT_EQ("", TURs.MainSourceFile);
  for (const auto& r : TURs.Replacements) {
    auto it = expectedReplacements.find(r);
    ASSERT_TRUE(it != expectedReplacements.end());
  }
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
