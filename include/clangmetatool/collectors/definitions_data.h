#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H

#include <string>
#include <unordered_map>
#include <clang/AST/Decl.h>

#include <clangmetatool/types/file_uid.h>

namespace clangmetatool {
namespace collectors {

class DefinitionsData {
    public:
    // uid of file that contains definition -> AST Node pointer
    std::unordered_multimap<types::FileUID, const clang::NamedDecl *> defs;
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H 
