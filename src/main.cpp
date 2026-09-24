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
        << "  huff decode <input> <output>\n"
        << "  huff info <file>\n";
}

void printRatio(double ratio) {
    std::cout << std::fixed << std::setprecision(2)
              << ratio * 100.0 << "%";
}

} // namespace

int main(int argc, char* argv[]) {
    if ((argc != 4 && argc != 3) ||
        (argc >= 2 && std::string(argv[1]) == "info" && argc != 3) ||
        (argc >= 2 && std::string(argv[1]) != "info" && argc != 4)) {
        printUsage();
        return 1;
    }

    const std::string command = argv[1];

    try {
        if (command == "encode") {
            const std::string input = argv[2];
            const std::string output = argv[3];

            const auto stats = compressFile(input, output);

            std::cout << "Encoded successfully.\n"
                      << "Original size:   " << stats.originalSize
                      << " bytes\n"
                      << "Compressed size: " << stats.compressedSize
                      << " bytes\n"
                      << "Ratio:           ";
            printRatio(stats.compressionRatio);
            std::cout << "\n";
        }
        else if (command == "decode") {
            const std::string input = argv[2];
            const std::string output = argv[3];

            const auto stats = decompressFile(input, output);

            std::cout << "Decoded successfully.\n"
                      << "Compressed size: " << stats.compressedSize
                      << " bytes\n"
                      << "Restored size:   " << stats.originalSize
                      << " bytes\n";
        }
        else if (command == "info") {
            const auto info = getFileInfo(argv[2]);

            std::cout << "Huffman file information\n"
                      << "-------------------------\n"
                      << "Original size:    " << info.originalSize
                      << " bytes\n"
                      << "Compressed size:  " << info.compressedSize
                      << " bytes\n"
                      << "Encoded bits:     " << info.encodedBits
                      << "\n"
                      << "Unique symbols:   " << info.uniqueSymbols
                      << "\n"
                      << "Compression ratio: ";
            printRatio(info.compressionRatio);
            std::cout << "\n";
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
