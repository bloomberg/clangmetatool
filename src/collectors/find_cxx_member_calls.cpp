#include <clangmetatool/collectors/find_cxx_member_calls.h>

#include <memory>
#include <string>

#include <clangmetatool/collectors/find_cxx_member_calls_data.h>

namespace clangmetatool {
namespace collectors {

using namespace clang::ast_matchers;

namespace {

class AnnotateCall1 : public clang::ast_matchers::MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  FindCXXMemberCallsData *data;

public:
  AnnotateCall1(clang::CompilerInstance *ci, FindCXXMemberCallsData *data)
      : ci(ci), data(data) {}

  virtual void
  run(const clang::ast_matchers::MatchFinder::MatchResult &r) override {

    const auto c = r.Nodes.getNodeAs<clang::CXXMemberCallExpr>("call");

    const auto f = r.Nodes.getNodeAs<clang::FunctionDecl>("context");

    data->insert(std::pair<const clang::FunctionDecl *,
                           const clang::CXXMemberCallExpr *>(f, c));
  }
};

} // namespace

class FindCXXMemberCallsImpl {
private:
  std::string c;
  std::string n;
  FindCXXMemberCallsData data;
  clang::CompilerInstance *ci;

  std::unique_ptr<StatementMatcher> sm;
  AnnotateCall1 cb1;

public:
  FindCXXMemberCallsImpl(clang::CompilerInstance *ci,
                         clang::ast_matchers::MatchFinder *f,
                         const std::string &c, const std::string &n)
      : ci(ci), c(c), n(n), cb1(ci, &data) {
    const auto cType = type(hasUnqualifiedDesugaredType(
        recordType(hasDeclaration(cxxRecordDecl(hasName(c))))));

    const auto cExpr =
        expr(anyOf(hasType(cType), hasType(qualType(pointsTo(cType)))));

    sm = std::make_unique<StatementMatcher>(
        cxxMemberCallExpr(on(cExpr), callee(cxxMethodDecl(hasName(n))),
                          hasAncestor(functionDecl().bind("context")))
            .bind("call"));

    f->addMatcher(*sm, &cb1);
  }

  ~FindCXXMemberCallsImpl() {}

  FindCXXMemberCallsData *getData() { return &data; }
};

FindCXXMemberCalls::FindCXXMemberCalls(clang::CompilerInstance *ci,
                                       clang::ast_matchers::MatchFinder *f,
                                       const std::string &c,
                                       const std::string &n) {
  impl = new FindCXXMemberCallsImpl(ci, f, c, n);
}

FindCXXMemberCalls::~FindCXXMemberCalls() { delete impl; }

FindCXXMemberCallsData *FindCXXMemberCalls::getData() {
  return impl->getData();
}
} // namespace collectors
} // namespace clangmetatool
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
