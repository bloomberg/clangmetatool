#ifndef INCLUDED_DEFS_DATA_H
#define INCLUDED_DEFS_DATA_H

#include <string>
#include <clang/AST/Decl.h>

namespace clangmetatool {
namespace collectors {

class DefData {
    public:
    // filename that contains definition -> AST Node
    std::unordered_multimap<std::string, const clang::NamedDecl *> defs;
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_DEFS_DATA_H 
