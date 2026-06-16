#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <string>
#include <unordered_map>

enum class Kind {
    STATIC,
    FIELD,
    ARG,
    LOCAL,
    NONE
};

class SymbolTable {
public:
    SymbolTable();
    void startSubroutine();
    void define(const std::string& name, const std::string& type, Kind kind);
    int varCount(Kind kind);
    Kind kindOf(const std::string& name);
    std::string typeOf(const std::string& name);
    int indexOf(const std::string& name);

private:
    struct IdentifierInfo {
        std::string type;
        Kind kind;
        int index;
    };
    std::unordered_map<std::string, IdentifierInfo> classScope;
    std::unordered_map<std::string, IdentifierInfo> subroutineScope;
    int staticIndex;
    int fieldIndex;
    int argIndex;
    int localIndex;
};
#endif