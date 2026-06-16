#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

// Represents an entry in the symbol table (e.g., "LOOP" -> 16)
typedef struct {
    char *name;
    int address;
} Symbol;

// The Parser object handles reading and processing the .asm file
typedef struct {
    FILE *inputFile;
    char **instructions;
    int instructionCount;
    int instructionCapacity;
    int currentIndex;
    int nextRamAddress; // For new variables
    Symbol *symbolTable;
    int symbolCount;
    int symbolCapacity;
} Parser;

// Public API
Parser *createParser(const char *filePath);
bool parserHasMoreCommands(Parser *parser);
char *parserAdvance(Parser *parser);
void destroyParser(Parser *parser);

#endif // PARSER_H