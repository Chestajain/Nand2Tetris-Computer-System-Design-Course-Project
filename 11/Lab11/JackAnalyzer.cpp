#include "CompilationEngine.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

void analyzeFile(const std::string& inputPath) {
    std::string outputPath = inputPath.substr(0, inputPath.find_last_of('.')) + ".vm";
    std::cout << "Compiling " << inputPath << " -> " << outputPath << std::endl;
    CompilationEngine engine(inputPath, outputPath);
    engine.compileClass();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: JackAnalyzer [file.jack | directory]" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".jack") {
                analyzeFile(entry.path().string());
            }
        }
    } else if (fs::is_regular_file(path) && fs::path(path).extension() == ".jack") {
        analyzeFile(path);
    } else {
        std::cerr << "Error: Invalid input. Please provide a .jack file or a directory." << std::endl;
        return 1;
    }

    return 0;
}