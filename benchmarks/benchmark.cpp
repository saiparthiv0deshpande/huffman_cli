#include "Compressor.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

namespace {

void writeTestFile(const fs::path& path, std::size_t size) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot create benchmark input");
    }

    const std::string pattern =
        "Huffman coding is a lossless compression algorithm. "
        "Repeated data should compress well. ";

    for (std::size_t written = 0; written < size;) {
        const std::size_t remaining = size - written;
        const std::size_t count =
            std::min(remaining, pattern.size());

        output.write(pattern.data(),
                     static_cast<std::streamsize>(count));
        written += count;
    }
}

void benchmark(
    const fs::path& directory,
    const std::string& name,
    std::size_t inputSize) {

    const fs::path input = directory / (name + ".input");
    const fs::path compressed = directory / (name + ".huff");
    const fs::path restored = directory / (name + ".output");

    writeTestFile(input, inputSize);

    const auto encodeStart = Clock::now();
    const auto encodeStats =
        compressFile(input.string(), compressed.string());
    const auto encodeEnd = Clock::now();

    const auto decodeStart = Clock::now();
    const auto decodeStats =
        decompressFile(compressed.string(), restored.string());
    const auto decodeEnd = Clock::now();

    const double encodeMs =
        std::chrono::duration<double, std::milli>(
            encodeEnd - encodeStart).count();

    const double decodeMs =
        std::chrono::duration<double, std::milli>(
            decodeEnd - decodeStart).count();

    const double encodeThroughput =
        encodeStats.originalSize / (encodeMs / 1000.0) /
        (1024.0 * 1024.0);

    const double decodeThroughput =
        decodeStats.originalSize / (decodeMs / 1000.0) /
        (1024.0 * 1024.0);

    std::cout << std::left
              << std::setw(12) << name
              << std::setw(15) << encodeStats.originalSize
              << std::setw(15) << encodeStats.compressedSize
              << std::setw(15) << std::fixed << std::setprecision(2)
              << encodeStats.compressionRatio * 100.0
              << std::setw(15) << encodeMs
              << std::setw(15) << decodeMs
              << std::setw(15) << encodeThroughput
              << decodeThroughput
              << "\n";

    fs::remove(input);
    fs::remove(compressed);
    fs::remove(restored);
}

} // namespace

int main() {
    try {
        const fs::path directory =
            fs::temp_directory_path() / "huffman_cli_benchmark";

        fs::create_directories(directory);

        std::cout << "Huffman CLI Benchmark\n";
        std::cout << "=====================\n\n";

        std::cout
            << std::left
            << std::setw(12) << "Dataset"
            << std::setw(15) << "Original(B)"
            << std::setw(15) << "Compressed(B)"
            << std::setw(15) << "Size(%)"
            << std::setw(15) << "Encode(ms)"
            << std::setw(15) << "Decode(ms)"
            << std::setw(15) << "Encode(MB/s)"
            << "Decode(MB/s)\n";

        std::cout << std::string(110, '-') << "\n";

        benchmark(directory, "1MB", 1 * 1024 * 1024);
        benchmark(directory, "5MB", 5 * 1024 * 1024);
        benchmark(directory, "10MB", 10 * 1024 * 1024);

        fs::remove_all(directory);
    }
    catch (const std::exception& ex) {
        std::cerr << "Benchmark failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
