#ifndef INCLUDED_DEFINITIONS_H
#define INCLUDED_DEFINITIONS_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clangmetatool/collectors/definitions_data.h>

namespace clangmetatool {
namespace collectors {

class DefinitionsImpl;

class Definitions {
private:
    DefinitionsImpl* impl;
public:
    Definitions(clang::CompilerInstance *ci, clang::ast_matchers::MatchFinder *f);
    ~Definitions();
    DefinitionsData* getData();
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_DEFINITIONS_H 
