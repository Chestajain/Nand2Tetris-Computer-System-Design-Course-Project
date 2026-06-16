#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "writer.h"

// The Assembler orchestrates the parsing and writing process.
typedef struct {
    char *inputFilePath;
    char *outputFilePath;
    Writer *writer;
} Assembler;

Assembler *createAssembler(const char *inputFilePath);
void runCompiler(Assembler *assembler);
void destroyAssembler(Assembler *assembler);

#endif // ASSEMBLER_H