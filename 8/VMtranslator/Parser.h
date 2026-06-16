#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <unordered_set>

class Parser {
public:
    // Enum for all possible command types
    enum CommandType {
        C_ARITHMETIC, C_PUSH, C_POP,
        C_LABEL, C_GOTO, C_IF,
        C_FUNCTION, C_CALL, C_RETURN
    };

    // Opens the input file/stream and gets ready to parse it.
    explicit Parser(const std::string& inputFilePath);

    // Are there more commands in the input?
    bool hasMoreCommands();

    // Reads the next command from the input and makes it the current command.
    void advance();

    // Returns the type of the current VM command.
    CommandType commandType();

    // Returns the first argument of the current command.
    std::string arg1();

    // Returns the second argument of the current command.
    int arg2();

private:
    size_t programCounter;
    std::vector<std::string> commandLines;
    
    // Fields for the currently parsed command
    CommandType currentCommandType;
    std::string parsedArg1;
    int parsedArg2;

    // Static set for quick lookup of arithmetic commands
    static std::unordered_set<std::string> arithmeticCommands;
};

#endif // PARSER_H