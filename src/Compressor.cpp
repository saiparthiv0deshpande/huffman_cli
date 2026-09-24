#include "Compressor.h"

#include "BitReader.h"
#include "BitWriter.h"
#include "FileFormat.h"
#include "HuffmanTree.h"

#include <array>
#include <fstream>
#include <limits>
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

uint64_t calculateEncodedBits(
    const std::array<uint64_t, 256>& frequencies,
    const std::unordered_map<uint8_t, std::string>& codes) {

    uint64_t encodedBits = 0;

    for (const auto& [byte, frequency] : frequencies) {
        if (frequency == 0) {
            continue;
        }

        const auto it = codes.find(byte);
        if (it == codes.end()) {
            throw std::runtime_error(
                "Internal error: missing Huffman code");
        }

        const uint64_t codeLength =
            static_cast<uint64_t>(it->second.size());

        if (frequency >
            (std::numeric_limits<uint64_t>::max() - encodedBits) /
                codeLength) {
            throw std::runtime_error("Encoded bit count overflow");
        }

        encodedBits += frequency * codeLength;
    }

    return encodedBits;
}

uint64_t payloadBytes(uint64_t encodedBits) {
    return encodedBits / 8 + (encodedBits % 8 != 0 ? 1 : 0);
}

} // namespace

CompressionStats compressFile(
    const std::string& inputPath,
    const std::string& outputPath) {

    if (inputPath == outputPath) {
        throw std::runtime_error(
            "Input and output paths must be different");
    }

    uint64_t originalSize = 0;
    const auto frequencies =
        buildFrequencyTable(inputPath, originalSize);

    HuffmanTree tree;
    tree.build(frequencies);

    const auto& codes = tree.getCodes();
    const uint64_t encodedBits =
        calculateEncodedBits(frequencies, codes);

    HuffFile::Header header;
    header.originalSize = originalSize;
    header.encodedBits = encodedBits;
    header.frequencies = frequencies;

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot reopen input file: " + inputPath);
    }

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error(
            "Cannot create output file: " + outputPath);
    }

    HuffFile::writeHeader(output, header);

    BitWriter writer(output);

    char ch;
    while (input.get(ch)) {
        const uint8_t byte =
            static_cast<uint8_t>(static_cast<unsigned char>(ch));

        const auto it = codes.find(byte);
        if (it == codes.end()) {
            throw std::runtime_error(
                "Internal error: missing Huffman code");
        }

        writer.writeBits(it->second);
    }

    writer.flush();
    output.flush();

    if (!output) {
        throw std::runtime_error("Failed to write compressed file");
    }

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

    if (inputPath == outputPath) {
        throw std::runtime_error(
            "Input and output paths must be different");
    }

    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error(
            "Cannot open compressed file: " + inputPath);
    }

    const HuffFile::Header header = HuffFile::readHeader(input);

    HuffmanTree tree;
    tree.build(header.frequencies);

    const uint64_t expectedPayloadBytes =
        payloadBytes(header.encodedBits);

    const auto payloadStart = input.tellg();
    if (payloadStart < 0) {
        throw std::runtime_error("Invalid compressed file position");
    }

    input.seekg(0, std::ios::end);
    const auto fileEnd = input.tellg();

    if (fileEnd < payloadStart ||
        static_cast<uint64_t>(fileEnd - payloadStart) !=
            expectedPayloadBytes) {
        throw std::runtime_error(
            "Compressed payload size does not match header");
    }

    input.seekg(payloadStart);

    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error(
            "Cannot create output file: " + outputPath);
    }

    if (header.originalSize == 0) {
        return {0, static_cast<uint64_t>(fileEnd), 0.0};
    }

    BitReader reader(input);
    uint64_t decodedBytes = 0;

    while (decodedBytes < header.originalSize) {
        uint8_t byte = 0;

        if (!tree.decodeByte(reader, byte)) {
            throw std::runtime_error(
                "Compressed data is truncated");
        }

        output.put(static_cast<char>(byte));
        if (!output) {
            throw std::runtime_error(
                "Failed to write decompressed file");
        }

        ++decodedBytes;
    }

    if (reader.bitsRead() != header.encodedBits) {
        throw std::runtime_error(
            "Encoded bit count does not match Huffman data");
    }

    bool paddingBit = false;
    while (reader.bitsRead() < expectedPayloadBytes * 8ULL) {
        if (!reader.readBit(paddingBit)) {
            throw std::runtime_error(
                "Compressed data is truncated");
        }

        if (paddingBit) {
            throw std::runtime_error(
                "Non-zero padding bits detected");
        }
    }

    output.flush();

    if (!output) {
        throw std::runtime_error(
            "Failed to finalize decompressed file");
    }

    CompressionStats stats;
    stats.originalSize = header.originalSize;
    stats.compressedSize = static_cast<uint64_t>(fileEnd);

    if (stats.originalSize > 0) {
        stats.compressionRatio =
            static_cast<double>(stats.compressedSize) /
            static_cast<double>(stats.originalSize);
    }

    return stats;
}

FileInfo getFileInfo(const std::string& inputPath) {
    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error(
            "Cannot open compressed file: " + inputPath);
    }

    const HuffFile::Header header = HuffFile::readHeader(input);
    const auto payloadStart = input.tellg();

    if (payloadStart < 0) {
        throw std::runtime_error("Invalid compressed file position");
    }

    input.seekg(0, std::ios::end);
    const auto end = input.tellg();

    if (end < payloadStart) {
        throw std::runtime_error("Invalid compressed file size");
    }

    const uint64_t expectedPayload =
        payloadBytes(header.encodedBits);

    const uint64_t actualPayload =
        static_cast<uint64_t>(end - payloadStart);

    if (actualPayload != expectedPayload) {
        throw std::runtime_error(
            "Compressed payload size does not match header");
    }

    FileInfo info;
    info.originalSize = header.originalSize;
    info.compressedSize = static_cast<uint64_t>(end);
    info.encodedBits = header.encodedBits;
    info.uniqueSymbols = header.uniqueSymbols;

    if (info.originalSize > 0) {
        info.compressionRatio =
            static_cast<double>(info.compressedSize) /
            static_cast<double>(info.originalSize);
    }

    return info;
}
