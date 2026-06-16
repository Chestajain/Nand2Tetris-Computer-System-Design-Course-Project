#include "Parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Helper to initialize the static command set.
static std::unordered_set<std::string> createArithmeticCommands() {
    return {"add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
}

// Initialize the static member variable.
std::unordered_set<std::string> Parser::arithmeticCommands = createArithmeticCommands();

// Constructor: Reads and cleans the input file.
Parser::Parser(const std::string& inputFilePath) : programCounter(0), parsedArg2(0) {
    std::ifstream sourceFile(inputFilePath);
    if (!sourceFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + inputFilePath);
    }

    std::string line;
    while (std::getline(sourceFile, line)) {
        // Remove comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (!line.empty()) {
            commandLines.push_back(line);
        }
    }
}

bool Parser::hasMoreCommands() {
    return programCounter < commandLines.size();
}

void Parser::advance() {
    if (!hasMoreCommands()) {
        return;
    }
    
    std::string currentFullCommand = commandLines[programCounter++];
    std::stringstream ss(currentFullCommand);
    std::vector<std::string> tokens;
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }

    const std::string& command = tokens[0];

    // Determine command type
    if (arithmeticCommands.count(command)) {
        currentCommandType = C_ARITHMETIC;
    } else if (command == "push")     currentCommandType = C_PUSH;
    else if (command == "pop")      currentCommandType = C_POP;
    else if (command == "label")    currentCommandType = C_LABEL;
    else if (command == "goto")     currentCommandType = C_GOTO;
    else if (command == "if-goto")  currentCommandType = C_IF;
    else if (command == "function") currentCommandType = C_FUNCTION;
    else if (command == "call")     currentCommandType = C_CALL;
    else if (command == "return")   currentCommandType = C_RETURN;
    else {
        throw std::runtime_error("Unrecognized command: " + command);
    }

    // Parse arguments
    if (currentCommandType == C_ARITHMETIC) {
        parsedArg1 = command;
    } else if (currentCommandType != C_RETURN) {
        if (tokens.size() > 1) parsedArg1 = tokens[1];
    }
    
    if (currentCommandType == C_PUSH || currentCommandType == C_POP || 
        currentCommandType == C_FUNCTION || currentCommandType == C_CALL) {
        if (tokens.size() > 2) parsedArg2 = std::stoi(tokens[2]);
    }
}

Parser::CommandType Parser::commandType() {
    return currentCommandType;
}

std::string Parser::arg1() {
    if (currentCommandType == C_RETURN) {
        throw std::runtime_error("arg1() cannot be called for a C_RETURN command.");
    }
    return parsedArg1;
}

int Parser::arg2() {
    if (currentCommandType != C_PUSH && currentCommandType != C_POP &&
        currentCommandType != C_FUNCTION && currentCommandType != C_CALL) {
        throw std::runtime_error("arg2() is only valid for PUSH, POP, FUNCTION, or CALL commands.");
    }
    return parsedArg2;
}