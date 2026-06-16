#ifndef WRITER_H
#define WRITER_H

#include "parser.h"
#include <stdio.h>

// The Writer object translates parsed commands into machine code.
typedef struct {
    Parser *parser;
    FILE *outputFile;
} Writer;

Writer *createWriter(const char *inputFilePath, const char *outputFilePath);
void writeMachineCode(Writer *writer);
void destroyWriter(Writer *writer);

#endif // WRITER_H