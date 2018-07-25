#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include <clangmetatool/collectors/references.h>

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
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::Stmt *st = r.Nodes.getNodeAs<clang::Stmt>("reference");
        if (st == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;
        const clang::FileID fid = sm->getFileID(st->getLocStart());
        const clang::FileEntry *entry = sm->getFileEntryForID(fid);
        if (!(entry && entry->isValid())) {
            return;
        }
        const types::FileUID fuid = entry->getUID();

        data->refs.insert(std::pair<types::FileUID, const clang::Stmt *>(fuid, st));
    }
};

} // close anonymous namespace

class ReferencesImpl {
private:
    ReferencesData data;

    StatementMatcher funcRefMatcher = callExpr().bind("reference");

    StatementMatcher varRefMatcher = declRefExpr().bind("reference");

    ReferencesDataAppender refAppender;

public:
    ReferencesImpl (clang::CompilerInstance *ci
            , MatchFinder *f)
        : refAppender(ci, &data)
    {
        f->addMatcher(funcRefMatcher, &refAppender);
        f->addMatcher(varRefMatcher, &refAppender);
    }

    ReferencesData* getData() {
        return &data;
    }
};

References::References
(clang::CompilerInstance *ci, MatchFinder *f) {
    impl = new ReferencesImpl(ci, f);
}

References::~References() {
    delete impl;
}

ReferencesData* References::getData() {
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
