#include <clangmetatool/collectors/definitions.h>

#include <clang/AST/Decl.h>

#include <clang/AST/Mangle.h>
#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/SourceManager.h>

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
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::NamedDecl *e = r.Nodes.getNodeAs<clang::NamedDecl>("def");
        if (e == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;
        const clang::FileID fileID = sm->getFileID(e->getLocation());

        data->defs.insert(std::pair<const clang::FileID, const clang::NamedDecl *>(fileID, e));
    }
};

} // close anonymous namespace

class DefinitionsImpl {
private:
    DefinitionsData data;

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

    DefinitionsDataAppender defAppender;

public:
    DefinitionsImpl (clang::CompilerInstance *ci
            , MatchFinder *f)
        : defAppender(ci, &data)
    {
        f->addMatcher(funcDefMatcher, &defAppender);
        f->addMatcher(varDefMatcher, &defAppender);
        f->addMatcher(classDefMatcher, &defAppender);
    }

    DefinitionsData* getData() {
        return &data;
    }
};

Definitions::Definitions
(clang::CompilerInstance *ci, MatchFinder *f) {
    impl = new DefinitionsImpl(ci, f);
}

Definitions::~Definitions() {
    delete impl;
}

DefinitionsData* Definitions::getData() {
    return impl->getData();
}

} // namespace collectors
} // namespace clangmetatool
