#include "SymbolTable.h"

SymbolTable::SymbolTable() : staticIndex(0), fieldIndex(0), argIndex(0), localIndex(0) {}

void SymbolTable::startSubroutine() {
    subroutineScope.clear();
    argIndex = 0;
    localIndex = 0;
}

void SymbolTable::define(const std::string& name, const std::string& type, Kind kind) {
    IdentifierInfo info;
    info.type = type;
    info.kind = kind;

    if (kind == Kind::STATIC) {
        info.index = staticIndex++;
        classScope[name] = info;
    } else if (kind == Kind::FIELD) {
        info.index = fieldIndex++;
        classScope[name] = info;
    } else if (kind == Kind::ARG) {
        info.index = argIndex++;
        subroutineScope[name] = info;
    } else if (kind == Kind::LOCAL) {
        info.index = localIndex++;
        subroutineScope[name] = info;
    }
}

int SymbolTable::varCount(Kind kind) {
    if (kind == Kind::STATIC) return staticIndex;
    if (kind == Kind::FIELD) return fieldIndex;
    if (kind == Kind::ARG) return argIndex;
    if (kind == Kind::LOCAL) return localIndex;
    return 0;
}

Kind SymbolTable::kindOf(const std::string& name) {
    if (subroutineScope.count(name)) {
        return subroutineScope[name].kind;
    }
    if (classScope.count(name)) {
        return classScope[name].kind;
    }
    return Kind::NONE;
}

std::string SymbolTable::typeOf(const std::string& name) {
    if (subroutineScope.count(name)) {
        return subroutineScope[name].type;
    }
    if (classScope.count(name)) {
        return classScope[name].type;
    }
    return "";
}

int SymbolTable::indexOf(const std::string& name) {
    if (subroutineScope.count(name)) {
        return subroutineScope[name].index;
    }
    if (classScope.count(name)) {
        return classScope[name].index;
    }
    return -1;
}