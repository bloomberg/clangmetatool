#ifndef INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H
#define INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H

#include <string>
#include <clang/AST/Decl.h>

namespace clangmetatool {
namespace collectors {

class FileIdHasher {
public:
    size_t operator() (const clang::FileID& fileID) const {
        return fileID.getHashValue();
    }
};

class DefinitionsData {
    public:
    // filename that contains definition -> AST Node
    std::unordered_multimap<const clang::FileID, const clang::NamedDecl *, FileIdHasher> defs;
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_CLANGMETATOOL_COLLECTORS_DEFINITIONS_DATA_H 
