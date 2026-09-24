#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <cstdint>
#include <string>

struct CompressionStats {
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    double compressionRatio = 0.0;
};

struct FileInfo {
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    uint64_t encodedBits = 0;
    uint16_t uniqueSymbols = 0;
    double compressionRatio = 0.0;
};

CompressionStats compressFile(
    const std::string& inputPath,
    const std::string& outputPath);

CompressionStats decompressFile(
    const std::string& inputPath,
    const std::string& outputPath);

FileInfo getFileInfo(const std::string& inputPath);

#endif
