#include "Parser.h"
#include "CodeWriter.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

// --- Helper Functions ---

/*
  @brief Collects all .vm file paths from a given input path (file or directory).
  Also determines the appropriate output .asm file name.
  @param inputPath The path to a .vm file or a directory.
  @param outputFile A reference to a string that will be set to the output file path.
  @return A vector of strings, each a path to a .vm file.
 */
std::vector<std::string> collectVmFiles(const std::string &inputPath, std::string &outputFile) {
    std::vector<std::string> vmFiles;
    fs::path pathObj(inputPath);

    if (fs::is_regular_file(pathObj)) {
        if (pathObj.extension() != ".vm") {
            throw std::invalid_argument("Input file must have a .vm extension");
        }
        vmFiles.push_back(pathObj.string());
        outputFile = pathObj.replace_extension(".asm").string();

    } else if (fs::is_directory(pathObj)) {
        for (const auto &entry : fs::directory_iterator(pathObj)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vm") {
                vmFiles.push_back(entry.path().string());
            }
        }
        if (vmFiles.empty()) {
            throw std::invalid_argument("No .vm files found in the specified directory");
        }
        std::string dirName = pathObj.filename().string();
        outputFile = (pathObj / (dirName + ".asm")).string();

    } else {
        throw std::invalid_argument("Invalid input path: not a file or a directory");
    }

    return vmFiles;
}

/**
 * @brief Processes a single .vm file, parsing it and generating assembly code.
 * @param vmFile The path to the .vm file.
 * @param writer A reference to the CodeWriter instance.
 */
void processFile(const std::string &vmFile, CodeWriter &writer) {
    Parser parser(vmFile);
    
    // Set the current file name in the CodeWriter for handling static variables
    writer.setFileName(fs::path(vmFile).stem().string());

    while (parser.hasMoreCommands()) {
        parser.advance();
        Parser::CommandType cmdType = parser.commandType();

        switch (cmdType) {
            case Parser::C_ARITHMETIC:
                writer.writeArithmetic(parser.arg1());
                break;
            case Parser::C_PUSH:
            case Parser::C_POP:
                writer.writePushPop(cmdType, parser.arg1(), parser.arg2());
                break;
            case Parser::C_LABEL:
                writer.writeLabel(parser.arg1());
                break;
            case Parser::C_GOTO:
                writer.writeGoto(parser.arg1());
                break;
            case Parser::C_IF:
                writer.writeIf(parser.arg1());
                break;
            case Parser::C_FUNCTION:
                writer.writeFunction(parser.arg1(), parser.arg2());
                break;
            case Parser::C_CALL:
                writer.writeCall(parser.arg1(), parser.arg2());
                break;
            case Parser::C_RETURN:
                writer.writeReturn();
                break;
        }
    }
}

// --- Main Execution ---

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: VMtranslator <inputfile.vm | inputdir>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputPath = argv[1];
    std::string outputFile;

    try {
        auto vmFiles = collectVmFiles(inputPath, outputFile);
        
        // A single CodeWriter handles all files.
        CodeWriter writer(outputFile);

        // Write bootstrap code only if translating a directory.
        if (fs::is_directory(inputPath)) {
            writer.writeInit();
        }

        // Process each file.
        for (const auto &file : vmFiles) {
            processFile(file, writer);
        }

        writer.close();
        std::cout << "Translation complete. Output written to: " << outputFile << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}