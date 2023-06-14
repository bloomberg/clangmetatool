#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>

#include <clangmetatool/collectors/definitions.h>

namespace clangmetatool {
namespace collectors {

namespace {

using namespace clang::ast_matchers;

class DefinitionsDataAppender : public MatchFinder::MatchCallback {
private:
  clang::CompilerInstance *ci;
  DefinitionsData *data;

public:
  DefinitionsDataAppender(clang::CompilerInstance *ci, DefinitionsData *data)
      : ci(ci), data(data) {}
  virtual void run(const MatchFinder::MatchResult &r) override {
    const clang::NamedDecl *e = r.Nodes.getNodeAs<clang::NamedDecl>("def");
    if (e == nullptr)
      return;

    clang::SourceManager *sm = r.SourceManager;
    const clang::FileID fid = sm->getFileID(e->getLocation());
#if LLVM_VERSION_MAJOR >= 15
    const llvm::Optional<clang::FileEntryRef> entry = sm->getFileEntryRefForID(fid);
    if (!entry.has_value()) {
      return;
    }
    const types::FileUID fuid = entry.value().getUID();
#else
    const clang::FileEntry *entry = sm->getFileEntryForID(fid);
    if (!(entry && entry->isValid())) {
      return;
    }
    const types::FileUID fuid = entry->getUID();
#endif

    data->defs.insert(
        std::pair<types::FileUID, const clang::NamedDecl *>(fuid, e));
  }
};

} // namespace

class DefinitionsImpl {
private:
  DefinitionsData data;

  DeclarationMatcher funcDefMatcher = functionDecl(isDefinition()).bind("def");

  DeclarationMatcher varDefMatcher =
      varDecl(allOf(isDefinition(), hasGlobalStorage())).bind("def");

  DeclarationMatcher classDefMatcher = recordDecl(isDefinition()).bind("def");

  DefinitionsDataAppender defAppender;

public:
  DefinitionsImpl(clang::CompilerInstance *ci, MatchFinder *f)
      : defAppender(ci, &data) {
    f->addMatcher(funcDefMatcher, &defAppender);
    f->addMatcher(varDefMatcher, &defAppender);
    f->addMatcher(classDefMatcher, &defAppender);
  }

  DefinitionsData *getData() { return &data; }
};

Definitions::Definitions(clang::CompilerInstance *ci, MatchFinder *f) {
  impl = new DefinitionsImpl(ci, f);
}

Definitions::~Definitions() { delete impl; }

DefinitionsData *Definitions::getData() { return impl->getData(); }

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
