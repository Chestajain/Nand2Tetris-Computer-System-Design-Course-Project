#include "CompilationEngine.h"
#include <iostream>
#include <unordered_map>
using namespace std;

static const unordered_map<Keyword, string> keywordToString = {
    {Keyword::CLASS, "class"}, {Keyword::CONSTRUCTOR, "constructor"},
    {Keyword::FUNCTION, "function"}, {Keyword::METHOD, "method"},
    {Keyword::FIELD, "field"}, {Keyword::STATIC, "static"},
    {Keyword::VAR, "var"}, {Keyword::INT, "int"},
    {Keyword::CHAR, "char"}, {Keyword::BOOLEAN, "boolean"},
    {Keyword::VOID, "void"}, {Keyword::TRUE, "true"},
    {Keyword::FALSE, "false"}, {Keyword::NULL_KEYWORD, "null"},
    {Keyword::THIS, "this"}, {Keyword::LET, "let"},
    {Keyword::DO, "do"}, {Keyword::IF, "if"},
    {Keyword::ELSE, "else"}, {Keyword::WHILE, "while"},
    {Keyword::RETURN, "return"}
};

static const unordered_map<char, string> symbolToEntity = {
    {'<', "&lt;"}, {'>', "&gt;"}, {'"', "&quot;"}, {'&', "&amp;"}
};


CompilationEngine::CompilationEngine(const string& inputFilename, const string& outputFilename)
    : tokenizer(inputFilename) {
    outputFile.open(outputFilename);
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open output file " << outputFilename << endl;
        exit(1);
    }
}

void CompilationEngine::writeXml(const string& tag, const string& value, bool isOpening) {
    string indent(indentationLevel * 2, ' ');
    if (isOpening) {
        outputFile << indent << "<" << tag << ">";
        if (!value.empty()) {
            outputFile << " " << value << " ";
            outputFile << "</" << tag << ">";
        }
        outputFile << endl;
        indentationLevel++;
    } else {
        indentationLevel--;
        indent = string(indentationLevel * 2, ' ');
        outputFile << indent << "</" << tag << ">" << endl;
    }
}


void CompilationEngine::writeTerminal(const string& tag, const string& value) {
    string indent(indentationLevel * 2, ' ');
    outputFile << indent << "<" << tag << "> " << value << " </" << tag << ">" << endl;
}

void CompilationEngine::process(const string& expected) {
    tokenizer.advance();
    switch (tokenizer.tokenType()) {
        case TokenType::KEYWORD:
            writeTerminal("keyword", keywordToString.at(tokenizer.keyword()));
            break;
        case TokenType::SYMBOL: {
            char s = tokenizer.symbol();
            if (symbolToEntity.count(s)) {
                writeTerminal("symbol", symbolToEntity.at(s));
            } else {
                writeTerminal("symbol", string(1, s));
            }
            break;
        }
        case TokenType::IDENTIFIER:
            writeTerminal("identifier", tokenizer.identifier());
            break;
        case TokenType::INT_CONST:
            writeTerminal("integerConstant", to_string(tokenizer.intVal()));
            break;
        case TokenType::STRING_CONST:
            writeTerminal("stringConstant", tokenizer.stringVal());
            break;
        default:
            break; 
    }
}


void CompilationEngine::compileClass() {
    writeXml("class", "", true);
    tokenizer.advance();
    writeTerminal("keyword", "class");

    processIdentifier(); 
    processSymbol('{');

    while (tokenizer.hasMoreTokens()) {
        auto nextToken = tokenizer.peekToken();
        if (nextToken.first != TokenType::KEYWORD) {
            break;
        }

        if (nextToken.second == "static" || nextToken.second == "field") {
            tokenizer.advance();
            compileClassVarDec();
        } else if (nextToken.second == "constructor" || nextToken.second == "function" || nextToken.second == "method") {
            tokenizer.advance();
            compileSubroutine();
        } else {
            break;
        }
    }

    processSymbol('}'); 
    writeXml("class", "", false);
}


void CompilationEngine::compileClassVarDec() {
    writeXml("classVarDec", "", true);
    writeTerminal("keyword", keywordToString.at(tokenizer.keyword()));
    
    compileType();
    
    processIdentifier();

    while (tokenizer.peekToken().second == ",") {
        processSymbol(','); 
        processIdentifier();
    }
    
    processSymbol(';');

    writeXml("classVarDec", "", false);
}

void CompilationEngine::compileSubroutine() {
    writeXml("subroutineDec", "", true);
    writeTerminal("keyword", keywordToString.at(tokenizer.keyword()));
    
    compileType(); 
    
    processIdentifier(); 

    processSymbol('(');
    compileParameterList();
    processSymbol(')');

    writeXml("subroutineBody", "", true);
    processSymbol('{');
    
    while(tokenizer.peekToken().second == "var") {
        tokenizer.advance();
        compileVarDec();
    }
    
    tokenizer.advance();
    compileStatements();

    processSymbol('}');
    writeXml("subroutineBody", "", false);
    writeXml("subroutineDec", "", false);
}

void CompilationEngine::compileParameterList() {
    writeXml("parameterList", "", true);
    if (tokenizer.peekToken().second != ")") {
        do {
           compileType();
           processIdentifier();
           if(tokenizer.peekToken().second != ",") {
               break;
           }
           processSymbol(',');
        } while(true);
    }
    writeXml("parameterList", "", false);
}


void CompilationEngine::compileVarDec() {
    writeXml("varDec", "", true);
    writeTerminal("keyword", "var");
    
    compileType();
    processIdentifier();

    while(tokenizer.peekToken().second == ",") {
        processSymbol(',');
        processIdentifier();
    }
    processSymbol(';');

    writeXml("varDec", "", false);
}

void CompilationEngine::compileStatements() {
    writeXml("statements", "", true);
    while (tokenizer.tokenType() == TokenType::KEYWORD) {
        Keyword kw = tokenizer.keyword();
        if (kw == Keyword::LET) compileLet();
        else if (kw == Keyword::IF) compileIf();
        else if (kw == Keyword::WHILE) compileWhile();
        else if (kw == Keyword::DO) compileDo();
        else if (kw == Keyword::RETURN) compileReturn();
        else break; 
        
        if (tokenizer.peekToken().second == "}"){
             break;
        }
        tokenizer.advance();
    }
    writeXml("statements", "", false);
}

void CompilationEngine::compileDo() {
    writeXml("doStatement", "", true);
    writeTerminal("keyword", "do");

    processIdentifier(); 

    if (tokenizer.peekToken().second == "(") { 
        processSymbol('(');
        compileExpressionList();
        processSymbol(')');
    } else {
        processSymbol('.');
        processIdentifier();
        processSymbol('(');
        compileExpressionList();
        processSymbol(')');
    }

    processSymbol(';');
    writeXml("doStatement", "", false);
}

void CompilationEngine::compileLet() {
    writeXml("letStatement", "", true);
    writeTerminal("keyword", "let"); 
    processIdentifier(); 

    if (tokenizer.peekToken().second == "[") {
        processSymbol('[');
        compileExpression();
        processSymbol(']');
    }

    processSymbol('=');
    compileExpression();
    processSymbol(';');

    writeXml("letStatement", "", false);
}


void CompilationEngine::compileWhile() {
    writeXml("whileStatement", "", true);
    writeTerminal("keyword", "while");
    processSymbol('(');
    compileExpression();
    processSymbol(')');
    processSymbol('{');
    tokenizer.advance();
    compileStatements();
    processSymbol('}');
    writeXml("whileStatement", "", false);
}


void CompilationEngine::compileReturn() {
    writeXml("returnStatement", "", true);
    writeTerminal("keyword", "return");
    if (tokenizer.peekToken().second != ";") {
        compileExpression();
    }
    processSymbol(';');
    writeXml("returnStatement", "", false);
}

void CompilationEngine::compileIf() {
    writeXml("ifStatement", "", true);
    writeTerminal("keyword", "if");
    processSymbol('(');
    compileExpression();
    processSymbol(')');
    processSymbol('{');
    tokenizer.advance();
    compileStatements();
    processSymbol('}');
    
    if(tokenizer.peekToken().second == "else") {
        tokenizer.advance();
        writeTerminal("keyword", "else");
        processSymbol('{');
        tokenizer.advance();
        compileStatements();
        processSymbol('}');
    }

    writeXml("ifStatement", "", false);
}

void CompilationEngine::compileExpression() {
    writeXml("expression", "", true);
    compileTerm();
    
    while(true) {
        auto next = tokenizer.peekToken();
        if (next.first == TokenType::SYMBOL) {
            char s = next.second[0];
            if (s == '+' || s == '-' || s == '*' || s == '/' || s == '&' || s == '|' || s == '<' || s == '>' || s == '=') {
                 tokenizer.advance();
                 if (symbolToEntity.count(s)) {
                    writeTerminal("symbol", symbolToEntity.at(s));
                } else {
                    writeTerminal("symbol", string(1, s));
                }
                compileTerm();
            } else {
                break;
            }
        } else {
            break; 
        }
    }

    writeXml("expression", "", false);
}

void CompilationEngine::compileTerm() {
    writeXml("term", "", true);
    tokenizer.advance();

    if (tokenizer.tokenType() == TokenType::INT_CONST) {
        writeTerminal("integerConstant", to_string(tokenizer.intVal()));
    } else if (tokenizer.tokenType() == TokenType::STRING_CONST) {
        writeTerminal("stringConstant", tokenizer.stringVal());
    } else if (tokenizer.tokenType() == TokenType::KEYWORD) {
        writeTerminal("keyword", keywordToString.at(tokenizer.keyword()));
    } else if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
        string identifier = tokenizer.identifier();
        
        auto next = tokenizer.peekToken();
        if (next.first == TokenType::SYMBOL && next.second == "[") {
            writeTerminal("identifier", identifier);
            processSymbol('[');
            compileExpression();
            processSymbol(']');
        } else if (next.first == TokenType::SYMBOL && next.second == ".") {
            writeTerminal("identifier", identifier);
            processSymbol('.');
            processIdentifier();
            processSymbol('(');
            compileExpressionList();
            processSymbol(')');
        } else if (next.first == TokenType::SYMBOL && next.second == "(") {
            writeTerminal("identifier", identifier);
            processSymbol('(');
            compileExpressionList();
            processSymbol(')');
        } else {
            writeTerminal("identifier", identifier);
        }
    } else if (tokenizer.tokenType() == TokenType::SYMBOL) {
        char s = tokenizer.symbol();
        if (s == '(') { 
            writeTerminal("symbol", "(");
            compileExpression();
            processSymbol(')');
        } else if (s == '-' || s == '~') { 
            writeTerminal("symbol", string(1, s));
            compileTerm();
        }
    }

    writeXml("term", "", false);
}


void CompilationEngine::compileExpressionList() {
    writeXml("expressionList", "", true);
    if (tokenizer.peekToken().second != ")") {
        compileExpression();
        while (tokenizer.peekToken().second == ",") {
            processSymbol(',');
            compileExpression();
        }
    }
    writeXml("expressionList", "", false);
}


void CompilationEngine::compileType(){
    tokenizer.advance();
    if(tokenizer.tokenType() == TokenType::KEYWORD && (tokenizer.keyword() == Keyword::INT || tokenizer.keyword() == Keyword::CHAR || tokenizer.keyword() == Keyword::BOOLEAN || tokenizer.keyword() == Keyword::VOID)){
        writeTerminal("keyword", keywordToString.at(tokenizer.keyword()));
    } else { 
        writeTerminal("identifier", tokenizer.identifier());
    }
}


void CompilationEngine::processSymbol(char expected) {
    tokenizer.advance();
    if (tokenizer.tokenType() == TokenType::SYMBOL && tokenizer.symbol() == expected) {
         if (symbolToEntity.count(expected)) {
            writeTerminal("symbol", symbolToEntity.at(expected));
        } else {
            writeTerminal("symbol", string(1, expected));
        }
    } else {
        cerr << "Error: Expected symbol '" << expected << "' but got '" << tokenizer.stringVal() << "'" << endl;
        exit(1);
    }
}

void CompilationEngine::processIdentifier() {
    tokenizer.advance();
    if (tokenizer.tokenType() == TokenType::IDENTIFIER) {
        writeTerminal("identifier", tokenizer.identifier());
    } else {
        cerr << "Error: Expected an identifier." << endl;
        exit(1);
    }
}

