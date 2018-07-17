#ifndef INCLUDED_DEFS_H
#define INCLUDED_DEFS_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clangmetatool/collectors/defs_data.h>

namespace clangmetatool {
namespace collectors {

class DefCollectorImpl;

class DefCollector {
private:
    DefCollectorImpl* impl;
public:
    DefCollector(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f);
    ~DefCollector();
    DefData* getData();
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_DEFS_H 
