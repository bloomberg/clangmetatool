#include <clangmetatool/collectors/member_method_decls.h>

namespace clangmetatool {
namespace collectors {

namespace {
class AnnotateMemberMethodDeclExpr
    : public clang::ast_matchers::MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  MemberMethodDeclsData *data;

public:
  AnnotateMemberMethodDeclExpr(clang::CompilerInstance *ci,
                               MemberMethodDeclsData *data)
      : ci(ci), data(data) {}

  virtual void
  run(const clang::ast_matchers::MatchFinder::MatchResult &r) override {
    const clang::CXXMethodDecl *d =
        r.Nodes.getNodeAs<clang::CXXMethodDecl>("decl");

    if (d == NULL) {
      return;
    }

    data->decls.insert(d);
  }
};
} // namespace

class MemberMethodDeclsImpl {
private:
  clang::CompilerInstance *ci;
  MemberMethodDeclsData data;
  clang::ast_matchers::DeclarationMatcher sm1 =
      clang::ast_matchers::cxxMethodDecl().bind("decl");
  AnnotateMemberMethodDeclExpr cb1;

public:
  MemberMethodDeclsImpl(clang::CompilerInstance *ci,
                        clang::ast_matchers::MatchFinder *f)
      : cb1(ci, &data) {
    f->addMatcher(sm1, &cb1);
  }
  MemberMethodDeclsData *getData() { return &data; }
};

MemberMethodDecls::MemberMethodDecls(clang::CompilerInstance *ci,
                                     clang::ast_matchers::MatchFinder *f) {
  impl = new MemberMethodDeclsImpl(ci, f);
}

MemberMethodDecls::~MemberMethodDecls() { delete impl; }

MemberMethodDeclsData *MemberMethodDecls::getData() { return impl->getData(); }
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
