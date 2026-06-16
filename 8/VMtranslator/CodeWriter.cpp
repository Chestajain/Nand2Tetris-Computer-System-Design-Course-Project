#include "CodeWriter.h"
#include <stdexcept>
#include <unordered_map>

CodeWriter::CodeWriter(const std::string& outputFilePath) : labelCounter(0) {
    assemblyFile.open(outputFilePath);
    if (!assemblyFile.is_open()) {
        throw std::runtime_error("Could not open output file: " + outputFilePath);
    }
}

void CodeWriter::setFileName(const std::string& fileName) {
    currentFileName = fileName;
}

void CodeWriter::writeInit() {
    assemblyFile << "// --- Bootstrap Code ---\n"
                 << "@256\n"
                 << "D=A\n"
                 << "@SP\n"
                 << "M=D\n"; // SP = 256
    writeCall("Sys.init", 0); // Call Sys.init
}

void CodeWriter::writeArithmetic(const std::string& command) {
    if (command == "add" || command == "sub" || command == "and" || command == "or") {
        assemblyFile << "@SP\nAM=M-1\nD=M\nA=A-1\n"; // Pop y, Pop x
        if (command == "add") assemblyFile << "M=M+D\n";
        else if (command == "sub") assemblyFile << "M=M-D\n";
        else if (command == "and") assemblyFile << "M=M&D\n";
        else if (command == "or") assemblyFile << "M=M|D\n";
    } else if (command == "neg" || command == "not") {
        assemblyFile << "@SP\nA=M-1\n"; // Point to top of stack
        if (command == "neg") assemblyFile << "M=-M\n";
        else if (command == "not") assemblyFile << "M=!M\n";
    } else if (command == "eq" || command == "gt" || command == "lt") {
        std::string trueLabel = generateUniqueLabel("JUMP_TRUE");
        std::string endLabel = generateUniqueLabel("JUMP_END");
        
        assemblyFile << "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n" // D = x-y
                     << "@" << trueLabel << "\n";
        if (command == "eq") assemblyFile << "D;JEQ\n";
        else if (command == "gt") assemblyFile << "D;JGT\n";
        else if (command == "lt") assemblyFile << "D;JLT\n";
        
        assemblyFile << "@SP\nA=M-1\nM=0\n" // False case
                     << "@" << endLabel << "\n0;JMP\n"
                     << "(" << trueLabel << ")\n"
                     << "@SP\nA=M-1\nM=-1\n" // True case
                     << "(" << endLabel << ")\n";
    }
}

void CodeWriter::writePushPop(Parser::CommandType command, const std::string& segment, int index) {
    static const std::unordered_map<std::string, std::string> segmentMap = {
        {"local", "LCL"}, {"argument", "ARG"}, {"this", "THIS"}, {"that", "THAT"}
    };

    if (command == Parser::C_PUSH) {
        if (segment == "constant") {
            assemblyFile << "@" << index << "\nD=A\n";
        } else if (segmentMap.count(segment)) {
            assemblyFile << "@" << segmentMap.at(segment) << "\nD=M\n@" << index << "\nA=D+A\nD=M\n";
        } else if (segment == "temp") {
            assemblyFile << "@R" << (5 + index) << "\nD=M\n";
        } else if (segment == "pointer") {
            assemblyFile << "@" << (index == 0 ? "THIS" : "THAT") << "\nD=M\n";
        } else if (segment == "static") {
            assemblyFile << "@" << currentFileName << "." << index << "\nD=M\n";
        }
        assemblyFile << "@SP\nA=M\nM=D\n@SP\nM=M+1\n"; // Push D to stack
    } else if (command == Parser::C_POP) {
        if (segmentMap.count(segment)) {
            assemblyFile << "@" << segmentMap.at(segment) << "\nD=M\n@" << index << "\nD=D+A\n@R13\nM=D\n"; // R13 = addr
            assemblyFile << "@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n"; // *addr = pop()
        } else if (segment == "temp") {
            assemblyFile << "@SP\nAM=M-1\nD=M\n@R" << (5 + index) << "\nM=D\n";
        } else if (segment == "pointer") {
            assemblyFile << "@SP\nAM=M-1\nD=M\n@" << (index == 0 ? "THIS" : "THAT") << "\nM=D\n";
        } else if (segment == "static") {
            assemblyFile << "@SP\nAM=M-1\nD=M\n@" << currentFileName << "." << index << "\nM=D\n";
        }
    }
}

void CodeWriter::writeLabel(const std::string& label) {
    assemblyFile << "(" << currentFunctionName << "$" << label << ")\n";
}

void CodeWriter::writeGoto(const std::string& label) {
    assemblyFile << "@" << currentFunctionName << "$" << label << "\n0;JMP\n";
}

void CodeWriter::writeIf(const std::string& label) {
    assemblyFile << "@SP\nAM=M-1\nD=M\n" // Pop value into D
                 << "@" << currentFunctionName << "$" << label << "\nD;JNE\n"; // If D != 0, jump
}

void CodeWriter::writeFunction(const std::string& functionName, int numLocals) {
    currentFunctionName = functionName;
    assemblyFile << "(" << functionName << ")\n";
    // Initialize local variables to 0
    for (int i = 0; i < numLocals; ++i) {
        writePushPop(Parser::C_PUSH, "constant", 0);
    }
}

void CodeWriter::writeCall(const std::string& functionName, int numArgs) {
    std::string returnLabel = generateUniqueLabel("RETURN");

    // push return-address
    assemblyFile << "@" << returnLabel << "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    // push LCL, ARG, THIS, THAT
    for (const char* seg : {"LCL", "ARG", "THIS", "THAT"}) {
        assemblyFile << "@" << seg << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    }
    // ARG = SP - numArgs - 5
    assemblyFile << "@SP\nD=M\n@5\nD=D-A\n@" << numArgs << "\nD=D-A\n@ARG\nM=D\n";
    // LCL = SP
    assemblyFile << "@SP\nD=M\n@LCL\nM=D\n";
    // goto functionName
    assemblyFile << "@" << functionName << "\n0;JMP\n";
    // (return-address)
    assemblyFile << "(" << returnLabel << ")\n";
}

void CodeWriter::writeReturn() {
    // FRAME = LCL (store in R13)
    assemblyFile << "@LCL\nD=M\n@R13\nM=D\n";
    // RET = *(FRAME - 5) (store in R14)
    assemblyFile << "@5\nA=D-A\nD=M\n@R14\nM=D\n";
    // *ARG = pop()
    assemblyFile << "@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n";
    // SP = ARG + 1
    assemblyFile << "@ARG\nD=M+1\n@SP\nM=D\n";
    // Restore THAT, THIS, ARG, LCL
    for (const char* seg : {"THAT", "THIS", "ARG", "LCL"}) {
        assemblyFile << "@R13\nAM=M-1\nD=M\n@" << seg << "\nM=D\n";
    }
    // goto RET
    assemblyFile << "@R14\nA=M\n0;JMP\n";
}

void CodeWriter::close() {
    if (assemblyFile.is_open()) {
        assemblyFile.close();
    }
}

std::string CodeWriter::generateUniqueLabel(const std::string& prefix) {
    return prefix + "_" + std::to_string(labelCounter++);
}