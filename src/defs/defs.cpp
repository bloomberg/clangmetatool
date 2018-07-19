#include <clangmetatool/collectors/defs.h>

#include <clang/AST/Decl.h>

#include <clang/AST/Mangle.h>
#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/SourceManager.h>

namespace clangmetatool {
namespace collectors {

namespace {

using namespace clang::ast_matchers;

class DefDataAppender : public MatchFinder::MatchCallback {
private:
    clang::CompilerInstance *ci;
    DefData *data;
public:
    DefDataAppender(clang::CompilerInstance *ci, DefData *data)
        : ci(ci), data(data) {}
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::NamedDecl *e = r.Nodes.getNodeAs<clang::NamedDecl>("def");
        if (e == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;
        const clang::FileID fileID = sm->getFileID(e->getLocation());

        data->defs.insert(std::pair<const clang::FileID, const clang::NamedDecl *>(fileID, e));
    }
};

} // close anonymous namespace

class DefCollectorImpl {
private:
    DefData data;

    DeclarationMatcher funcDefMatcher =
        functionDecl(
                isDefinition()
                ).bind("def");

    DeclarationMatcher varDefMatcher =
        varDecl(
                allOf(
                    isDefinition(), hasGlobalStorage()
                    )
               ).bind("def");


    DeclarationMatcher classDefMatcher =
        recordDecl(
                isDefinition()
                ).bind("def");

    DefDataAppender defAppender;

public:
    DefCollectorImpl (clang::CompilerInstance *ci
            , MatchFinder *f)
        : defAppender(ci, &data)
    {
        f->addMatcher(funcDefMatcher, &defAppender);
        f->addMatcher(varDefMatcher, &defAppender);
        f->addMatcher(classDefMatcher, &defAppender);
    }

    DefData* getData() {
        return &data;
    }
};

DefCollector::DefCollector
(clang::CompilerInstance *ci, MatchFinder *f) {
    impl = new DefCollectorImpl(ci, f);
}

DefCollector::~DefCollector() {
    delete impl;
}

DefData* DefCollector::getData() {
    return impl->getData();
}

} // namespace collectors
} // namespace clangmetatool
