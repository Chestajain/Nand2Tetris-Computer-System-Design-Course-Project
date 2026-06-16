#include "CompilationEngine.h"
#include <iostream>
#include <unordered_map>

using namespace std;

static const unordered_map<char, Command> opCommand = {
    {'+', Command::ADD}, {'-', Command::SUB}, {'*', Command::NONE},
    {'/', Command::NONE}, {'&', Command::AND}, {'|', Command::OR},
    {'<', Command::LT}, {'>', Command::GT}, {'=', Command::EQ}
};

static const unordered_map<char, Command> unaryOpCommand = {
    {'-', Command::NEG}, {'~', Command::NOT}
};

CompilationEngine::CompilationEngine(const string& inputFilename, const string& outputFilename)
    : tokenizer(inputFilename), vmWriter(outputFilename) {}

CompilationEngine::~CompilationEngine() {
    vmWriter.close();
}

string CompilationEngine::newLabel() {
    return "L" + to_string(labelIndex++);
}

void CompilationEngine::process(const string& expected) {
    tokenizer.advance();
}

void CompilationEngine::processSymbol(char expected) {
    tokenizer.advance();
    if (tokenizer.tokenType() != TokenType::SYMBOL || tokenizer.symbol() != expected) {
        cerr << "Error: Expected symbol '" << expected << "'" << endl;
        exit(1);
    }
}

void CompilationEngine::processKeyword(Keyword expected) {
    tokenizer.advance();
    if (tokenizer.tokenType() != TokenType::KEYWORD || tokenizer.keyword() != expected) {
        cerr << "Error: Expected keyword" << endl;
        exit(1);
    }
}

string CompilationEngine::processIdentifier() {
    tokenizer.advance();
    if (tokenizer.tokenType() != TokenType::IDENTIFIER) {
        cerr << "Error: Expected an identifier." << endl;
        exit(1);
    }
    return tokenizer.identifier();
}

string CompilationEngine::processType() {
    tokenizer.advance();
    if (tokenizer.tokenType() == TokenType::KEYWORD) {
        return tokenizer.stringVal();
    }
    if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
        return tokenizer.identifier();
    }
    cerr << "Error: Expected type (int, char, boolean, or className)" << endl;
    exit(1);
}

Kind CompilationEngine::keywordToKind(Keyword kw) {
    if (kw == Keyword::STATIC) return Kind::STATIC;
    if (kw == Keyword::FIELD) return Kind::FIELD;
    if (kw == Keyword::VAR) return Kind::LOCAL;
    return Kind::NONE;
}

Segment CompilationEngine::kindToSegment(Kind kind) {
    if (kind == Kind::STATIC) return Segment::STATIC;
    if (kind == Kind::FIELD) return Segment::THIS;
    if (kind == Kind::ARG) return Segment::ARG;
    if (kind == Kind::LOCAL) return Segment::LOCAL;
    return Segment::NONE;
}

void CompilationEngine::compileClass() {
    processKeyword(Keyword::CLASS);
    className = processIdentifier();
    processSymbol('{');

    while (tokenizer.peekToken().second == "static" || tokenizer.peekToken().second == "field") {
        compileClassVarDec();
    }

    while (tokenizer.peekToken().second == "constructor" || tokenizer.peekToken().second == "function" || tokenizer.peekToken().second == "method") {
        compileSubroutine();
    }

    processSymbol('}');
}

void CompilationEngine::compileClassVarDec() {
    tokenizer.advance(); 
    Kind kind = keywordToKind(tokenizer.keyword());
    string type = processType();
    string name = processIdentifier();
    symbolTable.define(name, type, kind);

    while (tokenizer.peekToken().second == ",") {
        processSymbol(',');
        name = processIdentifier();
        symbolTable.define(name, type, kind);
    }
    processSymbol(';');
}

void CompilationEngine::compileSubroutine() {
    symbolTable.startSubroutine();
    tokenizer.advance(); 
    Keyword kind = tokenizer.keyword();
    
    if (kind == Keyword::METHOD) {
        symbolTable.define("this", className, Kind::ARG);
    }

    processType(); 
    string subroutineName = processIdentifier();

    processSymbol('(');
    compileParameterList();
    processSymbol(')');
    compileSubroutineBody(kind, subroutineName);
}

void CompilationEngine::compileParameterList() {
    if (tokenizer.peekToken().second == ")") {
        return;
    }

    do {
        string type = processType();
        string name = processIdentifier();
        symbolTable.define(name, type, Kind::ARG);
    } while (tokenizer.peekToken().second == "," && (processSymbol(','), true));
}

void CompilationEngine::compileSubroutineBody(Keyword subroutineKind, const string& subroutineName) {
    processSymbol('{');
    
    while (tokenizer.peekToken().second == "var") {
        compileVarDec();
    }

    string fullSubroutineName = className + "." + subroutineName;
    vmWriter.writeFunction(fullSubroutineName, symbolTable.varCount(Kind::LOCAL));
    
    if (subroutineKind == Keyword::CONSTRUCTOR) {
        int nFields = symbolTable.varCount(Kind::FIELD);
        vmWriter.writePush(Segment::CONST, nFields);
        vmWriter.writeCall("Memory.alloc", 1);
        vmWriter.writePop(Segment::POINTER, 0);
    } else if (subroutineKind == Keyword::METHOD) {
        vmWriter.writePush(Segment::ARG, 0);
        vmWriter.writePop(Segment::POINTER, 0);
    }

    if (tokenizer.peekToken().second != "}") {
        compileStatements();
    }
    
    processSymbol('}');
}

void CompilationEngine::compileVarDec() {
    processKeyword(Keyword::VAR);
    string type = processType();
    string name = processIdentifier();
    symbolTable.define(name, type, Kind::LOCAL);

    while (tokenizer.peekToken().second == ",") {
        processSymbol(',');
        name = processIdentifier();
        symbolTable.define(name, type, Kind::LOCAL);
    }
    processSymbol(';');
}

void CompilationEngine::compileStatements() {
    while (tokenizer.peekToken().second != "}") {
        tokenizer.advance();
        Keyword kw = tokenizer.keyword();
        if (kw == Keyword::LET) compileLet();
        else if (kw == Keyword::IF) compileIf();
        else if (kw == Keyword::WHILE) compileWhile();
        else if (kw == Keyword::DO) compileDo();
        else if (kw == Keyword::RETURN) compileReturn();
        else break;
    }
}

void CompilationEngine::compileDo() {
    string name = processIdentifier();
    int nArgs = 0;

    if (tokenizer.peekToken().second == "(") {
        vmWriter.writePush(Segment::POINTER, 0);
        processSymbol('(');
        nArgs = compileExpressionList();
        processSymbol(')');
        vmWriter.writeCall(className + "." + name, nArgs + 1);
    } else {
        processSymbol('.');
        string subroutineName = processIdentifier();
        string type = symbolTable.typeOf(name);
        
        if (type.empty()) {
            name = name + "." + subroutineName;
        } else {
            vmWriter.writePush(kindToSegment(symbolTable.kindOf(name)), symbolTable.indexOf(name));
            name = type + "." + subroutineName;
            nArgs = 1;
        }
        
        processSymbol('(');
        nArgs += compileExpressionList();
        processSymbol(')');
        vmWriter.writeCall(name, nArgs);
    }

    vmWriter.writePop(Segment::TEMP, 0);
    processSymbol(';');
}

void CompilationEngine::compileLet() {
    string varName = processIdentifier();
    bool isArray = false;

    if (tokenizer.peekToken().second == "[") {
        isArray = true;
        Kind kind = symbolTable.kindOf(varName);
        vmWriter.writePush(kindToSegment(kind), symbolTable.indexOf(varName));
        processSymbol('[');
        compileExpression();
        processSymbol(']');
        vmWriter.writeArithmetic(Command::ADD);
    }

    processSymbol('=');
    compileExpression();
    processSymbol(';');

    if (isArray) {
        vmWriter.writePop(Segment::TEMP, 0);
        vmWriter.writePop(Segment::POINTER, 1);
        vmWriter.writePush(Segment::TEMP, 0);
        vmWriter.writePop(Segment::THAT, 0);
    } else {
        Kind kind = symbolTable.kindOf(varName);
        vmWriter.writePop(kindToSegment(kind), symbolTable.indexOf(varName));
    }
}

void CompilationEngine::compileWhile() {
    string labelStart = newLabel();
    string labelEnd = newLabel();
    
    vmWriter.writeLabel(labelStart);
    processSymbol('(');
    compileExpression();
    processSymbol(')');
    
    vmWriter.writeArithmetic(Command::NOT);
    vmWriter.writeIf(labelEnd);
    
    processSymbol('{');
    if (tokenizer.peekToken().second != "}") {
        compileStatements();
    }
    processSymbol('}');
    
    vmWriter.writeGoto(labelStart);
    vmWriter.writeLabel(labelEnd);
}

void CompilationEngine::compileReturn() {
    if (tokenizer.peekToken().second != ";") {
        compileExpression();
    } else {
        vmWriter.writePush(Segment::CONST, 0);
    }
    vmWriter.writeReturn();
    processSymbol(';');
}

void CompilationEngine::compileIf() {
    string labelElse = newLabel();
    string labelEnd = newLabel();

    processSymbol('(');
    compileExpression();
    processSymbol(')');
    
    vmWriter.writeArithmetic(Command::NOT);
    vmWriter.writeIf(labelElse); 

    processSymbol('{');
    if (tokenizer.peekToken().second != "}") {
        compileStatements();
    }
    processSymbol('}');
    
    vmWriter.writeGoto(labelEnd);
    vmWriter.writeLabel(labelElse);
    
    if (tokenizer.peekToken().second == "else") {
        processKeyword(Keyword::ELSE);
        processSymbol('{');
        if (tokenizer.peekToken().second != "}") {
            compileStatements();
        }
        processSymbol('}');
    }
    
    vmWriter.writeLabel(labelEnd);
}

void CompilationEngine::compileExpression() {
    compileTerm();
    
    while (opCommand.count(tokenizer.peekToken().second[0])) {
        tokenizer.advance();
        char op = tokenizer.symbol();
        compileTerm();
        
        if (op == '*') {
            vmWriter.writeCall("Math.multiply", 2);
        } else if (op == '/') {
            vmWriter.writeCall("Math.divide", 2);
        } else {
            vmWriter.writeArithmetic(opCommand.at(op));
        }
    }
}

void CompilationEngine::compileTerm() {
    tokenizer.advance();

    if (tokenizer.tokenType() == TokenType::INT_CONST) {
        vmWriter.writePush(Segment::CONST, tokenizer.intVal());
    } else if (tokenizer.tokenType() == TokenType::STRING_CONST) {
        string str = tokenizer.stringVal();
        vmWriter.writePush(Segment::CONST, str.length());
        vmWriter.writeCall("String.new", 1);
        for (char c : str) {
            vmWriter.writePush(Segment::CONST, (int)c);
            vmWriter.writeCall("String.appendChar", 2);
        }
    } else if (tokenizer.tokenType() == TokenType::KEYWORD) {
        Keyword kw = tokenizer.keyword();
        if (kw == Keyword::TRUE) {
            vmWriter.writePush(Segment::CONST, 0);
            vmWriter.writeArithmetic(Command::NOT);
        } else if (kw == Keyword::FALSE || kw == Keyword::NULL_KEYWORD) {
            vmWriter.writePush(Segment::CONST, 0);
        } else if (kw == Keyword::THIS) {
            vmWriter.writePush(Segment::POINTER, 0);
        }
    } else if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
        string name = tokenizer.identifier();
        
        auto next = tokenizer.peekToken();
        if (next.first == TokenType::SYMBOL && next.second == "[") {
            Kind kind = symbolTable.kindOf(name);
            vmWriter.writePush(kindToSegment(kind), symbolTable.indexOf(name));
            processSymbol('[');
            compileExpression();
            processSymbol(']');
            vmWriter.writeArithmetic(Command::ADD);
            vmWriter.writePop(Segment::POINTER, 1);
            vmWriter.writePush(Segment::THAT, 0);
        } else if (next.first == TokenType::SYMBOL && (next.second == "." || next.second == "(")) {
            int nArgs = 0;
            if (next.second == "(") {
                vmWriter.writePush(Segment::POINTER, 0);
                processSymbol('(');
                nArgs = compileExpressionList();
                processSymbol(')');
                vmWriter.writeCall(className + "." + name, nArgs + 1);
            } else {
                processSymbol('.');
                string subroutineName = processIdentifier();
                string type = symbolTable.typeOf(name);
                
                if (type.empty()) {
                    name = name + "." + subroutineName;
                } else {
                    vmWriter.writePush(kindToSegment(symbolTable.kindOf(name)), symbolTable.indexOf(name));
                    name = type + "." + subroutineName;
                    nArgs = 1;
                }
                
                processSymbol('(');
                nArgs += compileExpressionList();
                processSymbol(')');
                vmWriter.writeCall(name, nArgs);
            }
        } else {
            Kind kind = symbolTable.kindOf(name);
            vmWriter.writePush(kindToSegment(kind), symbolTable.indexOf(name));
        }
    } else if (tokenizer.tokenType() == TokenType::SYMBOL) {
        char s = tokenizer.symbol();
        if (s == '(') {
            compileExpression();
            processSymbol(')');
        } else if (unaryOpCommand.count(s)) {
            compileTerm();
            vmWriter.writeArithmetic(unaryOpCommand.at(s));
        }
    }
}

int CompilationEngine::compileExpressionList() {
    int nArgs = 0;
    if (tokenizer.peekToken().second != ")") {
        compileExpression();
        nArgs = 1;
        while (tokenizer.peekToken().second == ",") {
            processSymbol(',');
            compileExpression();
            nArgs++;
        }
    }
    return nArgs;
}