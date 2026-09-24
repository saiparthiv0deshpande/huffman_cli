#include "Compressor.h"

#include "BitReader.h"
#include "BitWriter.h"
#include "FileFormat.h"
#include "HuffmanTree.h"

#include <array>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

std::array<uint64_t, 256> buildFrequencyTable(
    const std::string& path,
    uint64_t& fileSize) {

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open input file: " + path);
    }

    std::array<uint64_t, 256> frequencies{};
    fileSize = 0;

    char ch;
    while (input.get(ch)) {
        const uint8_t byte =
            static_cast<uint8_t>(static_cast<unsigned char>(ch));
        ++frequencies[byte];
        ++fileSize;
    }

    if (!input.eof()) {
        throw std::runtime_error("Failed while reading input file");
    }

    return frequencies;
}

} // namespace

CompressionStats compressFile(
    const std::string& inputPath,
    const std::string& outputPath) {

    uint64_t originalSize = 0;
    const auto frequencies =
        buildFrequencyTable(inputPath, originalSize);

    HuffmanTree tree;
    tree.build(frequencies);

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot create output file: " + outputPath);
    }

    HuffFile::Header header;
    header.originalSize = originalSize;
    header.frequencies = frequencies;

    // Write a placeholder header. Padding is updated by writing it as part
    // of the final format only after encoding, so we buffer the encoded
    // payload in a temporary stream below.
    std::ostringstream payload;
    BitWriter writer(
        reinterpret_cast<std::ofstream&>(
            *static_cast<std::ofstream*>(nullptr)));

    (void)writer;
    (void)payload;

    throw std::runtime_error(
        "Internal implementation checkpoint: encoder payload wiring pending");
}

CompressionStats decompressFile(
    const std::string& inputPath,
    const std::string& outputPath) {

    (void)inputPath;
    (void)outputPath;

    throw std::runtime_error(
        "Internal implementation checkpoint: decoder wiring pending");
}
