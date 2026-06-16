#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"
#include <string>
#include <fstream>
#include <vector>

class CompilationEngine {
public:
    CompilationEngine(const std::string& inputFilename, const std::string& outputFilename);
    ~CompilationEngine();
    void compileClass();

private:
    JackTokenizer tokenizer;
    SymbolTable symbolTable;
    VMWriter vmWriter;
    std::string className;
    int labelIndex = 0;

    std::string newLabel();
    void process(const std::string& expected);
    void processSymbol(char expected);
    void processKeyword(Keyword expected);
    std::string processIdentifier();
    std::string processType();
    
    void compileClassVarDec();
    void compileSubroutine();
    void compileParameterList();
    void compileSubroutineBody(Keyword subroutineKind, const std::string& subroutineName);
    void compileVarDec();
    void compileStatements();
    void compileDo();
    void compileLet();
    void compileWhile();
    void compileReturn();
    void compileIf();
    void compileExpression();
    void compileTerm();
    int compileExpressionList();
    
    Kind keywordToKind(Keyword kw);
    Segment kindToSegment(Kind kind);
};

#endif