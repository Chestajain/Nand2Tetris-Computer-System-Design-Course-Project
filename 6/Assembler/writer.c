#include "writer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Lookup table entry for a mnemonic and its binary representation
typedef struct { const char *mnemonic, *bits; } MnemonicEntry;

// Mnemonic Lookup Tables
static MnemonicEntry destMnemonics[] = {
    {"", "000"}, {"M", "001"}, {"D", "010"}, {"MD", "011"},
    {"A", "100"}, {"AM", "101"}, {"AD", "110"}, {"AMD", "111"},
};
static MnemonicEntry compMnemonics[] = {
    {"0", "0101010"}, {"1", "0111111"}, {"-1", "0111010"},
    {"D", "0001100"}, {"A", "0110000"}, {"!D", "0001101"},
    {"!A", "0110001"}, {"-D", "0001111"}, {"-A", "0110011"},
    {"D+1", "0011111"}, {"A+1", "0110111"}, {"D-1", "0001110"},
    {"A-1", "0110010"}, {"D+A", "0000010"}, {"D-A", "0010011"},
    {"A-D", "0000111"}, {"D&A", "0000000"}, {"D|A", "0010101"},
    {"M", "1110000"}, {"!M", "1110001"}, {"-M", "1110011"},
    {"M+1", "1110111"}, {"M-1", "1110010"}, {"D+M", "1000010"},
    {"D-M", "1010011"}, {"M-D", "1000111"}, {"D&M", "1000000"},
    {"D|M", "1010101"},
};
static MnemonicEntry jumpMnemonics[] = {
    {"", "000"}, {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"},
    {"JLT", "100"}, {"JNE", "101"}, {"JLE", "110"}, {"JMP", "111"},
};

//Translation Logic
// Looks up a mnemonic in a given table and returns its binary code.
static const char *lookupMnemonic(const MnemonicEntry *table, size_t size, const char *mnemonic) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp(table[i].mnemonic, mnemonic) == 0) {
            return table[i].bits;
        }
    }
    return NULL;
}

// Translates a single assembly instruction into a 16-bit binary string.
static char *translateInstruction(const char *instruction) {
    char *binaryString = malloc(17);
    if (!binaryString) exit(EXIT_FAILURE);
    binaryString[16] = '\0';

    // A-Instruction: @value
    if (instruction[0] == '@') {
        int value = atoi(instruction + 1);
        for (int i = 15; i >= 0; i--) {
            binaryString[i] = (value & 1) ? '1' : '0';
            value >>= 1;
        }
        return binaryString;
    }

    // C-Instruction: dest=comp;jump
    char destStr[8] = "", compStr[8] = "", jumpStr[8] = "";
    char instructionCopy[256];
    strcpy(instructionCopy, instruction);

    char *jumpPart = strchr(instructionCopy, ';');
    if (jumpPart) {
        *jumpPart = '\0'; // Split the string at the semicolon
        strcpy(jumpStr, jumpPart + 1);
    }

    char *destPart = strchr(instructionCopy, '=');
    if (destPart) {
        *destPart = '\0'; // Split the string at the equals sign
        strcpy(destStr, instructionCopy);
        strcpy(compStr, destPart + 1);
    } else {
        strcpy(compStr, instructionCopy);
    }

    const char *compBits = lookupMnemonic(compMnemonics, sizeof(compMnemonics)/sizeof(MnemonicEntry), compStr);
    const char *destBits = lookupMnemonic(destMnemonics, sizeof(destMnemonics)/sizeof(MnemonicEntry), destStr);
    const char *jumpBits = lookupMnemonic(jumpMnemonics, sizeof(jumpMnemonics)/sizeof(MnemonicEntry), jumpStr);

    if (!compBits || !destBits || !jumpBits) {
        fprintf(stderr, "Error: Invalid C-instruction '%s'\n", instruction);
        exit(EXIT_FAILURE);
    }

    snprintf(binaryString, 17, "111%s%s%s", compBits, destBits, jumpBits);
    return binaryString;
}

// Public API Implementation 
Writer *createWriter(const char *inputFilePath, const char *outputFilePath) {
    Writer *writer = malloc(sizeof(Writer));
    if (!writer) exit(EXIT_FAILURE);

    writer->parser = createParser(inputFilePath);
    writer->outputFile = fopen(outputFilePath, "w");
    if (!writer->outputFile) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return writer;
}

void writeMachineCode(Writer *writer) {
    while (parserHasMoreCommands(writer->parser)) {
        char *instruction = parserAdvance(writer->parser);
        char *binaryCode = translateInstruction(instruction);
        fprintf(writer->outputFile, "%s\n", binaryCode);
        free(binaryCode);
    }
}

void destroyWriter(Writer *writer) {
    if (!writer) return;
    fclose(writer->outputFile);
    destroyParser(writer->parser);
    free(writer);
}