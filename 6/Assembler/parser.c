#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128
#define LINE_BUFFER_SIZE 256

// Forward declarations for static helper functions
static void firstPass(Parser *parser);
static void secondPass(Parser *parser);

// String Manipulation Utilities 

// Removes leading/trailing whitespace from a string, in-place.
static char *trimWhitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Removes comments from a line, in-place.
static void stripComments(char *str) {
    char *comment = strstr(str, "//");
    if (comment) *comment = '\0';
}

// Symbol Table Management

// Adds a new symbol-address pair to the symbol table.
static void addSymbol(Parser *parser, const char *name, int address) {
    if (parser->symbolCount == parser->symbolCapacity) {
        parser->symbolCapacity *= 2;
        parser->symbolTable = realloc(parser->symbolTable, parser->symbolCapacity * sizeof(Symbol));
    }
    parser->symbolTable[parser->symbolCount].name = strdup(name);
    parser->symbolTable[parser->symbolCount].address = address;
    parser->symbolCount++;
}

// Finds a symbol in the table and returns its address, or -1 if not found.
static int findSymbolAddress(Parser *parser, const char *name) {
    for (int i = 0; i < parser->symbolCount; i++) {
        if (strcmp(parser->symbolTable[i].name, name) == 0) {
            return parser->symbolTable[i].address;
        }
    }
    return -1;
}

// Initializes the symbol table with predefined symbols.
static void initializePredefinedSymbols(Parser *parser) {
    for (int i = 0; i < 16; i++) {
        char r_name[4];
        snprintf(r_name, sizeof(r_name), "R%d", i);
        addSymbol(parser, r_name, i);
    }
    addSymbol(parser, "SP", 0);
    addSymbol(parser, "LCL", 1);
    addSymbol(parser, "ARG", 2);
    addSymbol(parser, "THIS", 3);
    addSymbol(parser, "THAT", 4);
    addSymbol(parser, "SCREEN", 16384);
    addSymbol(parser, "KBD", 24576);
}

//Main Parsing Logic 

// First pass: Find all (LABEL) declarations and add them to the symbol table.
static void firstPass(Parser *parser) {
    char line[LINE_BUFFER_SIZE];
    int romAddress = 0;
    while (fgets(line, sizeof(line), parser->inputFile)) {
        stripComments(line);
        char *instruction = trimWhitespace(line);
        if (*instruction == '\0') continue;

        if (instruction[0] == '(') {
            char *end = strchr(instruction, ')');
            if (end) {
                *end = '\0';
                addSymbol(parser, instruction + 1, romAddress);
            }
        } else {
            romAddress++;
        }
    }
}

// Second pass: Process instructions, resolve symbols, and build the final command list.
static void secondPass(Parser *parser) {
    char line[LINE_BUFFER_SIZE];
    rewind(parser->inputFile);

    while (fgets(line, sizeof(line), parser->inputFile)) {
        stripComments(line);
        char *instruction = trimWhitespace(line);
        if (*instruction == '\0' || instruction[0] == '(') continue;

        if (parser->instructionCount == parser->instructionCapacity) {
            parser->instructionCapacity *= 2;
            parser->instructions = realloc(parser->instructions, parser->instructionCapacity * sizeof(char *));
        }

        char *final_instruction;
        if (instruction[0] == '@') {
            char *symbol = instruction + 1;
            if (isdigit((unsigned char)symbol[0])) {
                final_instruction = strdup(instruction);
            } else {
                int address = findSymbolAddress(parser, symbol);
                if (address == -1) {
                    address = parser->nextRamAddress++;
                    addSymbol(parser, symbol, address);
                }
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "@%d", address);
                final_instruction = strdup(buffer);
            }
        } else {
            final_instruction = strdup(instruction);
        }
        parser->instructions[parser->instructionCount++] = final_instruction;
    }
}

// Public API Implementation 

Parser *createParser(const char *filePath) {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->instructionCapacity = INITIAL_CAPACITY;
    parser->instructions = malloc(parser->instructionCapacity * sizeof(char *));
    parser->symbolCapacity = INITIAL_CAPACITY;
    parser->symbolTable = malloc(parser->symbolCapacity * sizeof(Symbol));
    parser->nextRamAddress = 16; // Start allocating variables at RAM[16]

    initializePredefinedSymbols(parser);

    parser->inputFile = fopen(filePath, "r");
    if (!parser->inputFile) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    firstPass(parser);
    secondPass(parser);

    return parser;
}

bool parserHasMoreCommands(Parser *parser) {
    return parser->currentIndex < parser->instructionCount;
}

char *parserAdvance(Parser *parser) {
    if (parserHasMoreCommands(parser)) {
        return parser->instructions[parser->currentIndex++];
    }
    return NULL;
}

void destroyParser(Parser *parser) {
    if (!parser) return;
    fclose(parser->inputFile);
    for (int i = 0; i < parser->instructionCount; i++) {
        free(parser->instructions[i]);
    }
    free(parser->instructions);
    for (int i = 0; i < parser->symbolCount; i++) {
        free(parser->symbolTable[i].name);
    }
    free(parser->symbolTable);
    free(parser);
}