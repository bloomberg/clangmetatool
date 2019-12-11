#include <clang/Basic/SourceManager.h>

#include <clangmetatool/collectors/references.h>

#include <utility>

namespace clangmetatool {
namespace collectors {

namespace {

using namespace clang::ast_matchers;

class ReferencesDataAppender : public MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  ReferencesData *data;

public:
  ReferencesDataAppender(clang::CompilerInstance *ci, ReferencesData *data)
      : ci(ci), data(data) {}
  virtual void run(const MatchFinder::MatchResult &r) override {
    const clang::NamedDecl *ref =
        r.Nodes.getNodeAs<clang::NamedDecl>("reference");
    if (ref == nullptr)
      return;

    const clang::NamedDecl *ctxt =
        r.Nodes.getNodeAs<clang::NamedDecl>("context");
    if (ctxt == nullptr)
      return;

    data->refs.insert(std::make_pair(ref, ctxt));
    data->deps.insert(std::make_pair(ctxt, ref));
  }
};

} // namespace

class ReferencesImpl {
private:
  ReferencesData data;

  StatementMatcher funcContextMatcher = anyOf(
      // matches vars and funcs
      declRefExpr(
          allOf(hasAncestor(functionDecl(isDefinition()).bind("context")),
                hasDeclaration(anyOf(
                    varDecl(allOf(unless(parmVarDecl()), hasGlobalStorage()))
                        .bind("reference"),
                    functionDecl().bind("reference"))))),
      // matches C++ classes from constructor
      cxxConstructExpr(
          allOf(hasAncestor(functionDecl(isDefinition()).bind("context")),
                hasDeclaration(namedDecl().bind("reference")))));

  StatementMatcher varContextMatcher = anyOf(
      // matches vars and funcs
      declRefExpr(
          allOf(hasAncestor(varDecl(allOf(isDefinition(), hasGlobalStorage()))
                                .bind("context")),
                hasDeclaration(anyOf(varDecl().bind("reference"),
                                     functionDecl().bind("reference"))))),
      // matches C++ classes from constructor
      cxxConstructExpr(
          allOf(hasAncestor(varDecl(allOf(isDefinition(), hasGlobalStorage()))
                                .bind("context")),
                hasDeclaration(namedDecl().bind("reference")))));

  DeclarationMatcher funcInRecordMatcher =
      recordDecl(
          allOf(isDefinition(), hasDescendant(functionDecl().bind("reference"))

                    ))
          .bind("context");

  DeclarationMatcher fieldInRecordMatcher =
      recordDecl(
          allOf(isDefinition(), hasDescendant(fieldDecl().bind("reference"))))
          .bind("context");

  ReferencesDataAppender refAppender;

public:
  ReferencesImpl(clang::CompilerInstance *ci, MatchFinder *f)
      : refAppender(ci, &data) {
    f->addMatcher(funcContextMatcher, &refAppender);
    f->addMatcher(varContextMatcher, &refAppender);
    f->addMatcher(funcInRecordMatcher, &refAppender);
    f->addMatcher(fieldInRecordMatcher, &refAppender);
  }

  ReferencesData *getData() { return &data; }
};

References::References(clang::CompilerInstance *ci, MatchFinder *f) {
  impl = new ReferencesImpl(ci, f);
}

References::~References() { delete impl; }

ReferencesData *References::getData() { return impl->getData(); }

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
