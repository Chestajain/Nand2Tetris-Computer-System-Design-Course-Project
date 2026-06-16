#ifndef CODEWRITER_H
#define CODEWRITER_H

#include "Parser.h"
#include <fstream>
#include <string>

class CodeWriter {
public:
    // Opens the output file/stream.
    explicit CodeWriter(const std::string& outputFilePath);

    // Informs the CodeWriter that a new VM file has started.
    void setFileName(const std::string& fileName);
    
    // Writes assembly code that effects the VM initialization (bootstrap).
    void writeInit();

    // Writes the assembly code for the given arithmetic command.
    void writeArithmetic(const std::string& command);

    // Writes the assembly code for a C_PUSH or C_POP command.
    void writePushPop(Parser::CommandType command, const std::string& segment, int index);

    // --- New Methods for Project 8 ---
    void writeLabel(const std::string& label);
    void writeGoto(const std::string& label);
    void writeIf(const std::string& label);
    void writeFunction(const std::string& functionName, int numLocals);
    void writeCall(const std::string& functionName, int numArgs);
    void writeReturn();

    // Closes the output file.
    void close();

private:
    std::ofstream assemblyFile;
    std::string currentFileName;
    std::string currentFunctionName;
    int labelCounter;

    // Helper to generate a unique label
    std::string generateUniqueLabel(const std::string& prefix);
};

#endif // CODEWRITER_H