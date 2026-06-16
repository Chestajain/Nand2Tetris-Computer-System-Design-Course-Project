#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generates an output file path by replacing ".asm" with ".hack".
// Returns a newly allocated string, or NULL on error.
static char *generateOutputFilePath(const char *inputPath) {
    const char *extension = strrchr(inputPath, '.');
    if (!extension || strcmp(extension, ".asm") != 0) {
        return NULL;
    }
    size_t baseLength = extension - inputPath;
    char *outputPath = malloc(baseLength + 6); // .hack + null terminator
    if (!outputPath) return NULL;

    memcpy(outputPath, inputPath, baseLength);
    strcpy(outputPath + baseLength, ".hack");
    return outputPath; 
}

Assembler *createAssembler(const char *inputFilePath) {
    char *outputFilePath = generateOutputFilePath(inputFilePath);
    if (!outputFilePath) {
        fprintf(stderr, "Error: Input file must have a .asm extension.\n");
        return NULL;
    }

    Assembler *assembler = malloc(sizeof(Assembler));
    if (!assembler) {
        free(outputFilePath);
        return NULL;
    }

    assembler->inputFilePath = strdup(inputFilePath);
    assembler->outputFilePath = outputFilePath;
    assembler->writer = createWriter(assembler->inputFilePath, assembler->outputFilePath);
    return assembler;
}

void runCompiler(Assembler *assembler) {
    if (!assembler) return;
    writeMachineCode(assembler->writer);
}

void destroyAssembler(Assembler *assembler) {
    if (!assembler) return;
    destroyWriter(assembler->writer);
    free(assembler->inputFilePath);
    free(assembler->outputFilePath);
    free(assembler);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.asm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Assembler *assembler = createAssembler(argv[1]);
    if (!assembler) {
        return EXIT_FAILURE;
    }

    printf("Assembling %s -> %s\n", assembler->inputFilePath, assembler->outputFilePath);
    runCompiler(assembler);
    destroyAssembler(assembler);
    printf("Assembly complete.\n");

    return EXIT_SUCCESS;
}