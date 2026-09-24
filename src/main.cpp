#include "Compressor.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

void printUsage() {
    std::cout
        << "Usage:\n"
        << "  huff encode <input> <output>\n"
        << "  huff decode <input> <output>\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    const std::string command = argv[1];
    const std::string input = argv[2];
    const std::string output = argv[3];

    try {
        if (command == "encode") {
            const auto stats = compressFile(input, output);

            std::cout << "Encoded successfully.\n"
                      << "Original size:   " << stats.originalSize
                      << " bytes\n"
                      << "Compressed size: " << stats.compressedSize
                      << " bytes\n"
                      << "Ratio:           "
                      << std::fixed << std::setprecision(2)
                      << stats.compressionRatio * 100.0
                      << "%\n";
        }
        else if (command == "decode") {
            const auto stats = decompressFile(input, output);

            std::cout << "Decoded successfully.\n"
                      << "Compressed size: " << stats.compressedSize
                      << " bytes\n"
                      << "Restored size:   " << stats.originalSize
                      << " bytes\n";
        }
        else {
            printUsage();
            return 1;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
