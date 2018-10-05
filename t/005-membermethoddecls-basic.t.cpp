#include "clangmetatool-testconfig.h"

#include <gtest/gtest.h>

#include <clangmetatool/collectors/member_method_decls.h>
#include <clangmetatool/collectors/member_method_decls_data.h>
#include <clangmetatool/meta_tool.h>
#include <clangmetatool/meta_tool_factory.h>

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Core/Replacement.h>
#include <clang/Tooling/Refactoring.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/CommandLine.h>

#include <iostream>

class MyTool {
private:
  clang::CompilerInstance *ci;
  clangmetatool::collectors::MemberMethodDecls v;

public:
  MyTool(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f)
      : ci(ci), v(ci, f) {}
  void postProcessing(
      std::map<std::string, clang::tooling::Replacements> &replacementsMap) {

    std::set<const clang::CXXMethodDecl *> *decls = &(v.getData()->decls);

    ASSERT_EQ(4, decls->size()) << "Found decl ref";

    const char *names[] = {"count", "update", "run", "count"};
    const char *parents[] = {"MyObject", "MyObject", "OurObject", "MyObject"};

    int count = 0;
    auto it = decls->begin();
    while (it != decls->end()) {
      const clang::CXXMethodDecl *decl = *it;

      ASSERT_EQ(std::string(names[count]), decl->getNameAsString())
          << "Got the expected method name";

      ASSERT_EQ(std::string(parents[count]),
                decl->getParent()->getNameAsString())
          << "Got the expected parent struct name";

      count++;
      it++;
    }
  }
};

TEST(use_meta_tool, factory) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");

  int argc = 4;
  const char *argv[] = {"foo", CMAKE_SOURCE_DIR
                        "/t/data/005-membermethoddecls-basic/foo.cpp",
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
