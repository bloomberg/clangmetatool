#ifndef INCLUDED_DEFS_DATA_H
#define INCLUDED_DEFS_DATA_H

namespace clangmetatool {
namespace collectors {

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

#endif //INCLUDED_DEFS_DATA_H 
