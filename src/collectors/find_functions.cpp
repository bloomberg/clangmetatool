#include <clangmetatool/collectors/find_functions.h>

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clangmetatool {
namespace collectors {

using namespace clang::ast_matchers;

namespace {

class AnnotateFunction : public MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  FindFunctionsData *data;

public:
  AnnotateFunction(clang::CompilerInstance *ci, FindFunctionsData *data)
      : ci(ci), data(data) {}

  virtual void run(const MatchFinder::MatchResult &r) override {
    const clang::FunctionDecl *f =
        r.Nodes.getNodeAs<clang::FunctionDecl>("func");

    // Don't match macros/macro expansions or things declared extern
    if (!f->getBeginLoc().isMacroID() &&
        clang::SC_Extern != f->getStorageClass()) {
      data->push_back(f);
    }
  }
};

} // anonymous namespace

class FindFunctionsImpl {
private:
  FindFunctionsData data;

  DeclarationMatcher dm = functionDecl(isExpansionInMainFile()).bind("func");

  AnnotateFunction af;

public:
  FindFunctionsImpl(clang::CompilerInstance *ci,
                    clang::ast_matchers::MatchFinder *f)
      : af(ci, &data) {
    f->addMatcher(dm, &af);
  }

  const FindFunctionsData *getData() const { return &data; }
};

FindFunctions::FindFunctions(clang::CompilerInstance *ci,
                             clang::ast_matchers::MatchFinder *f) {
  impl = new FindFunctionsImpl(ci, f);
}

FindFunctions::~FindFunctions() { delete impl; }

const FindFunctionsData *FindFunctions::getData() const {
  return impl->getData();
}

} // namespace collectors
} // namespace clangmetatool

// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
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
