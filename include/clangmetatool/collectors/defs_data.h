#ifndef INCLUDED_DEFS_DATA_H
#define INCLUDED_DEFS_DATA_H

#include <string>

namespace clangmetatool {
namespace collectors {

class SymbolData {
    public:
        std::string type;

        SymbolData() : type("") {}
        SymbolData(const std::string &type) : type(type) {}
};

class DefData {
    public:
    // mangled symbol name -> useful stuff
    std::unordered_map<std::string, SymbolData> defs;
};

} // namespace collectors
} // namespace clangmetatool

#endif //INCLUDED_DEFS_DATA_H 
