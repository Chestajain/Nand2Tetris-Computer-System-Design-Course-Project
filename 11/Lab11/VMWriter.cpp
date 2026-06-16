#include "VMWriter.h"
#include <iostream>

VMWriter::VMWriter(const std::string& outputFilename) {
    outputFile.open(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFilename << std::endl;
        exit(1);
    }
}

void VMWriter::writePush(Segment segment, int index) {
    outputFile << "push " << segmentToString(segment) << " " << index << std::endl;
}

void VMWriter::writePop(Segment segment, int index) {
    outputFile << "pop " << segmentToString(segment) << " " << index << std::endl;
}

void VMWriter::writeArithmetic(Command command) {
    outputFile << commandToString(command) << std::endl;
}

void VMWriter::writeLabel(const std::string& label) {
    outputFile << "label " << label << std::endl;
}

void VMWriter::writeGoto(const std::string& label) {
    outputFile << "goto " << label << std::endl;
}

void VMWriter::writeIf(const std::string& label) {
    outputFile << "if-goto " << label << std::endl;
}

void VMWriter::writeCall(const std::string& name, int nArgs) {
    outputFile << "call " << name << " " << nArgs << std::endl;
}

void VMWriter::writeFunction(const std::string& name, int nLocals) {
    outputFile << "function " << name << " " << nLocals << std::endl;
}

void VMWriter::writeReturn() {
    outputFile << "return" << std::endl;
}

void VMWriter::close() {
    outputFile.close();
}

std::string VMWriter::segmentToString(Segment segment) {
    switch (segment) {
        case Segment::CONST:   return "constant";
        case Segment::ARG:     return "argument";
        case Segment::LOCAL:   return "local";
        case Segment::STATIC:  return "static";
        case Segment::THIS:    return "this";
        case Segment::THAT:    return "that";
        case Segment::POINTER: return "pointer";
        case Segment::TEMP:    return "temp";
        default:               return "";
    }
}

std::string VMWriter::commandToString(Command command) {
    switch (command) {
        case Command::ADD: return "add";
        case Command::SUB: return "sub";
        case Command::NEG: return "neg";
        case Command::EQ:  return "eq";
        case Command::GT:  return "gt";
        case Command::LT:  return "lt";
        case Command::AND: return "and";
        case Command::OR:  return "or";
        case Command::NOT: return "not";
        default:           return "";
    }
}