#ifndef INCLUDED_DEF_COLLECTOR_H
#define INCLUDED_DEF_COLLECTOR_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clangmetatool {
namespace collectors {

class DefCollectorImpl;
class DefData;

class DefCollector {
private:
    DefCollectorImpl* impl;
public:
    DefCollector(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f);
    ~DefCollector();
    DefData* getData();
};

class SymbolData {
    public:
    // TODO : src location, mangled name, demangled name, 
    int tmp;
};

class DefData {
    public:
    // mangled symbol name -> useful stuff
    std::unordered_map<std::string, SymbolData> defs;
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_DEF_COLLECTOR_H 
