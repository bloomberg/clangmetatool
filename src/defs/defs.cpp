#include <clangmetatool/collectors/defs.h>

#include <clang/AST/Decl.h>

#include <clang/AST/Mangle.h>
#include <llvm/Support/raw_ostream.h>

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
        // TODO : insert def info into 'data'
        const clang::NamedDecl *e = r.Nodes.getNodeAs<clang::NamedDecl>("def");
        if (e == nullptr) return;

        std::string mangled_symbol;
        llvm::raw_string_ostream mangled_sym_adapter(mangled_symbol);
        clang::ItaniumMangleContext* mangler_context_p = clang::ItaniumMangleContext::create(ci->getASTContext(), ci->getDiagnostics());

        mangler_context_p->mangleName(e, mangled_sym_adapter);
        data->defs[mangled_symbol] = SymbolData();
    }
};

} // close anonymous namespace

class DefCollectorImpl {
private:
    DefData data;

    // TODO :
    // This will need to be narrowed to definitions
    // NamedDecl doesn't have a "isThisDeclarationADefinition", so
    // we can't use the narrowing matcher of isDefinition() here;
    // will likely have to be split up into more specific types:
    // - FunctionDecl
    // - VarDecl
    // not sure how class definitions work yet or what "TagDecl" is
    DeclarationMatcher defMatcher = namedDecl().bind("def");
    DefDataAppender defAppender;

public:
    DefCollectorImpl (clang::CompilerInstance *ci
            , MatchFinder *f)
        : defAppender(ci, &data) 
    {
        f->addMatcher(defMatcher, &defAppender);
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
