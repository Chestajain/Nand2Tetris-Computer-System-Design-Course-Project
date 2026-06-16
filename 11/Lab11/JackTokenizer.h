#ifndef JACK_TOKENIZER_H
#define JACK_TOKENIZER_H

#include <string>
#include <fstream>
#include <vector>
#include <utility> 

enum class TokenType {
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST,
    NONE 
};

enum class Keyword {
    CLASS, CONSTRUCTOR, FUNCTION, METHOD,
    FIELD, STATIC, VAR,
    INT, CHAR, BOOLEAN, VOID,
    TRUE, FALSE, NULL_KEYWORD, THIS,
    LET, DO, IF, ELSE, WHILE, RETURN
};

class JackTokenizer {
public:
    explicit JackTokenizer(const std::string& input_filename);

    bool hasMoreTokens();

    void advance();

    TokenType tokenType();

    Keyword keyword();

    char symbol();

    std::string identifier();

    int intVal();

    std::string stringVal();

    std::pair<TokenType, std::string> peekToken();

    JackTokenizer(const JackTokenizer&) = delete;
    JackTokenizer& operator=(const JackTokenizer&) = delete;

private:
    std::ifstream inputFile;
    std::string currentToken;
    TokenType currentTokenType;

    void processToken();
    void cleanFile();
    std::string fileContent;
    size_t cursor = 0;
};

#endif