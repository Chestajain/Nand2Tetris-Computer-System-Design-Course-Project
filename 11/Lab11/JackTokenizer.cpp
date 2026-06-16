#include "JackTokenizer.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>

static const std::unordered_map<std::string, Keyword> keywordMap = {
    {"class", Keyword::CLASS}, {"constructor", Keyword::CONSTRUCTOR},
    {"function", Keyword::FUNCTION}, {"method", Keyword::METHOD},
    {"field", Keyword::FIELD}, {"static", Keyword::STATIC},
    {"var", Keyword::VAR}, {"int", Keyword::INT},
    {"char", Keyword::CHAR}, {"boolean", Keyword::BOOLEAN},
    {"void", Keyword::VOID}, {"true", Keyword::TRUE},
    {"false", Keyword::FALSE}, {"null", Keyword::NULL_KEYWORD},
    {"this", Keyword::THIS}, {"let", Keyword::LET},
    {"do", Keyword::DO}, {"if", Keyword::IF},
    {"else", Keyword::ELSE}, {"while", Keyword::WHILE},
    {"return", Keyword::RETURN}
};

static const std::string symbols = "{}()[].,;+-*/&|<>=~";

JackTokenizer::JackTokenizer(const std::string& input_filename) {
    inputFile.open(input_filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << input_filename << std::endl;
        exit(1);
    }
    cleanFile();
    currentTokenType = TokenType::NONE;
}

void JackTokenizer::cleanFile() {
    std::string line;
    std::string content;
    bool inBlockComment = false;

    while (getline(inputFile, line)) {
        std::string cleanedLine;
        for (size_t i = 0; i < line.length(); ++i) {
            if (inBlockComment) {
                if (i + 1 < line.length() && line[i] == '*' && line[i+1] == '/') {
                    inBlockComment = false;
                    i++; 
                }
            } else {
                if (i + 1 < line.length() && line[i] == '/' && line[i+1] == '/') {
                    break; 
                }
                if (i + 1 < line.length() && line[i] == '/' && line[i+1] == '*') {
                    inBlockComment = true;
                    i++;
                } else {
                    cleanedLine += line[i];
                }
            }
        }
        if (!cleanedLine.empty()) {
            size_t first = cleanedLine.find_first_not_of(" \t\n\r");
            if (std::string::npos != first) {
                size_t last = cleanedLine.find_last_not_of(" \t\n\r");
                content += cleanedLine.substr(first, (last - first + 1)) + " ";
            }
        }
    }
    fileContent = content;
}


bool JackTokenizer::hasMoreTokens() {
    while (cursor < fileContent.length() && isspace(fileContent[cursor])) {
        cursor++;
    }
    return cursor < fileContent.length();
}

void JackTokenizer::advance() {
    if (!hasMoreTokens()) {
        currentTokenType = TokenType::NONE;
        return;
    }

    char firstChar = fileContent[cursor];

    if (isdigit(firstChar)) { 
        currentToken = "";
        while (cursor < fileContent.length() && isdigit(fileContent[cursor])) {
            currentToken += fileContent[cursor];
            cursor++;
        }
        currentTokenType = TokenType::INT_CONST;
    } else if (symbols.find(firstChar) != std::string::npos) { 
        currentToken = firstChar;
        cursor++;
        currentTokenType = TokenType::SYMBOL;
    } else if (firstChar == '"') {
        cursor++; 
        currentToken = "";
        while (cursor < fileContent.length() && fileContent[cursor] != '"') {
            currentToken += fileContent[cursor];
            cursor++;
        }
        cursor++;
        currentTokenType = TokenType::STRING_CONST;
    } else {
        currentToken = "";
        while (cursor < fileContent.length() && (isalnum(fileContent[cursor]) || fileContent[cursor] == '_')) {
            currentToken += fileContent[cursor];
            cursor++;
        }
        if (keywordMap.count(currentToken)) {
            currentTokenType = TokenType::KEYWORD;
        } else {
            currentTokenType = TokenType::IDENTIFIER;
        }
    }
}

std::pair<TokenType, std::string> JackTokenizer::peekToken() {
    size_t temp_cursor = cursor;
    
    while (temp_cursor < fileContent.length() && isspace(fileContent[temp_cursor])) {
        temp_cursor++;
    }

    if (temp_cursor >= fileContent.length()) {
        return {TokenType::NONE, ""};
    }

    char firstChar = fileContent[temp_cursor];
    std::string peeked_token_str;
    TokenType peeked_type;

    if (isdigit(firstChar)) {
        while (temp_cursor < fileContent.length() && isdigit(fileContent[temp_cursor])) {
            peeked_token_str += fileContent[temp_cursor];
            temp_cursor++;
        }
        peeked_type = TokenType::INT_CONST;
    } else if (symbols.find(firstChar) != std::string::npos) {
        peeked_token_str = firstChar;
        peeked_type = TokenType::SYMBOL;
    } else if (firstChar == '"') {
        temp_cursor++; 
        while (temp_cursor < fileContent.length() && fileContent[temp_cursor] != '"') {
            peeked_token_str += fileContent[temp_cursor];
            temp_cursor++;
        }
        peeked_type = TokenType::STRING_CONST;
    } else {
        while (temp_cursor < fileContent.length() && (isalnum(fileContent[temp_cursor]) || fileContent[temp_cursor] == '_')) {
            peeked_token_str += fileContent[temp_cursor];
            temp_cursor++;
        }
        if (keywordMap.count(peeked_token_str)) {
            peeked_type = TokenType::KEYWORD;
        } else {
            peeked_type = TokenType::IDENTIFIER;
        }
    }
    return {peeked_type, peeked_token_str};
}
TokenType JackTokenizer::tokenType() {
    return currentTokenType;
}
Keyword JackTokenizer::keyword() {
    return keywordMap.at(currentToken);
}
char JackTokenizer::symbol() {
    return currentToken[0];
}
std::string JackTokenizer::identifier() {
    return currentToken;
}
int JackTokenizer::intVal() {
    return std::stoi(currentToken);
}
std::string JackTokenizer::stringVal() {
    return currentToken;
}