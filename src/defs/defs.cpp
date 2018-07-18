#include <clangmetatool/collectors/defs.h>

#include <clang/AST/Decl.h>

#include <clang/AST/Mangle.h>
#include <llvm/Support/raw_ostream.h>

#include <clang/Basic/SourceManager.h>

namespace clangmetatool {
namespace collectors {

namespace {

using namespace clang::ast_matchers;

// TODO : We can probably consolidate the callbacks:
#if 0
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

        std::string filename = std::string(sm->getFilename(e->getLocation()));

        data->defs.insert(std::pair<std::string, const clang::NamedDecl *>(filename, e));
    }
};
#endif

class FuncDefDataAppender : public MatchFinder::MatchCallback {
private:
    clang::CompilerInstance *ci;
    DefData *data;
public:
    FuncDefDataAppender(clang::CompilerInstance *ci, DefData *data)
        : ci(ci), data(data) {}
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::FunctionDecl *e = r.Nodes.getNodeAs<clang::FunctionDecl>("def");
        if (e == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;

        std::string filename = std::string(sm->getFilename(e->getLocation()));

        data->defs.insert(std::pair<std::string, const clang::NamedDecl *>(filename, e));
    }
};

class VarDefDataAppender : public MatchFinder::MatchCallback {
private:
    clang::CompilerInstance *ci;
    DefData *data;
public:
    VarDefDataAppender(clang::CompilerInstance *ci, DefData *data)
        : ci(ci), data(data) {}
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::VarDecl *e = r.Nodes.getNodeAs<clang::VarDecl>("def");
        if (e == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;

        std::string filename = std::string(sm->getFilename(e->getLocation()));

        data->defs.insert(std::pair<std::string, const clang::NamedDecl *>(filename, e));
    }
};

class ClassDefDataAppender : public MatchFinder::MatchCallback {
private:
    clang::CompilerInstance *ci;
    DefData *data;
public:
    ClassDefDataAppender(clang::CompilerInstance *ci, DefData *data)
        : ci(ci), data(data) {}
    virtual void run(const MatchFinder::MatchResult & r) override {
        const clang::CXXRecordDecl *e = r.Nodes.getNodeAs<clang::CXXRecordDecl>("def");
        if (e == nullptr) return;

        clang::SourceManager *sm = r.SourceManager;

        std::string filename = std::string(sm->getFilename(e->getLocation()));

        data->defs.insert(std::pair<std::string, const clang::NamedDecl *>(filename, e));
    }

};

} // close anonymous namespace

class DefCollectorImpl {
private:
    DefData data;

    DeclarationMatcher funcDefMatcher =
        functionDecl
        (isDefinition()
        ).bind("def");
    FuncDefDataAppender funcDefAppender;

    DeclarationMatcher varDefMatcher =
        varDecl(
                allOf(
                    isDefinition(), hasGlobalStorage()
                    )
               ).bind("def");
    VarDefDataAppender varDefAppender;


    DeclarationMatcher classDefMatcher =
        recordDecl(
                isDefinition()
                ).bind("def");
    ClassDefDataAppender classDefAppender;

public:
    DefCollectorImpl (clang::CompilerInstance *ci
            , MatchFinder *f)
        : funcDefAppender(ci, &data)
          , varDefAppender(ci, &data)
          , classDefAppender(ci, &data)
    {
        f->addMatcher(funcDefMatcher, &funcDefAppender);
        f->addMatcher(varDefMatcher, &varDefAppender);
        f->addMatcher(classDefMatcher, &classDefAppender);
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
