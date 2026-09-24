#include "Compressor.h"

#include "BitReader.h"
#include "BitWriter.h"
#include "FileFormat.h"
#include "HuffmanTree.h"

#include <fstream>
#include <sstream>
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

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot reopen input file: " + inputPath);
    }

    // Build the payload in memory first so the header can contain
    // the exact number of padding bits.
    std::ostringstream payload;
    BitWriter writer(payload);

    const auto& codes = tree.getCodes();

    char ch;
    while (input.get(ch)) {
        const uint8_t byte =
            static_cast<uint8_t>(static_cast<unsigned char>(ch));

        auto it = codes.find(byte);
        if (it == codes.end()) {
            throw std::runtime_error("Internal error: missing Huffman code");
        }

        writer.writeBits(it->second);
    }

    const uint8_t paddingBits = writer.flush();

    HuffFile::Header header;
    header.originalSize = originalSize;
    header.frequencies = frequencies;
    header.paddingBits = paddingBits;

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot create output file: " + outputPath);
    }

    HuffFile::writeHeader(output, header);

    const std::string encoded = payload.str();
    output.write(encoded.data(),
                 static_cast<std::streamsize>(encoded.size()));

    if (!output) {
        throw std::runtime_error("Failed to write compressed file");
    }

    output.flush();

    CompressionStats stats;
    stats.originalSize = originalSize;

    const auto end = output.tellp();
    stats.compressedSize =
        end >= 0 ? static_cast<uint64_t>(end) : 0;

    if (stats.originalSize > 0) {
        stats.compressionRatio =
            static_cast<double>(stats.compressedSize) /
            static_cast<double>(stats.originalSize);
    }

    return stats;
}

CompressionStats decompressFile(
    const std::string& inputPath,
    const std::string& outputPath) {

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open compressed file: " + inputPath);
    }

    const HuffFile::Header header = HuffFile::readHeader(input);

    HuffmanTree tree;
    tree.build(header.frequencies);

    const auto& codes = tree.getCodes();

    std::unordered_map<std::string, uint8_t> reverseCodes;
    for (const auto& [byte, code] : codes) {
        reverseCodes.emplace(code, byte);
    }

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot create output file: " + outputPath);
    }

    if (header.originalSize == 0) {
        return {0, 0, 0.0};
    }

    if (reverseCodes.empty()) {
        throw std::runtime_error("Compressed file contains no Huffman codes");
    }

    BitReader reader(input);
    std::string currentCode;
    uint64_t decodedBytes = 0;
    bool bit = false;

    while (decodedBytes < header.originalSize &&
           reader.readBit(bit)) {

        currentCode.push_back(bit ? '1' : '0');

        auto it = reverseCodes.find(currentCode);
        if (it != reverseCodes.end()) {
            output.put(static_cast<char>(it->second));
            if (!output) {
                throw std::runtime_error("Failed to write decompressed file");
            }

            ++decodedBytes;
            currentCode.clear();
        }
    }

    if (decodedBytes != header.originalSize || !currentCode.empty()) {
        throw std::runtime_error(
            "Compressed data is truncated or corrupted");
    }

    output.flush();

    CompressionStats stats;
    stats.originalSize = header.originalSize;

    input.seekg(0, std::ios::end);
    const auto end = input.tellg();
    stats.compressedSize =
        end >= 0 ? static_cast<uint64_t>(end) : 0;

    if (stats.originalSize > 0) {
        stats.compressionRatio =
            static_cast<double>(stats.compressedSize) /
            static_cast<double>(stats.originalSize);
    }

    return stats;
}
