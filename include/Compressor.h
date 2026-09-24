#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <cstdint>
#include <string>

struct CompressionStats {
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    double compressionRatio = 0.0;
};

CompressionStats compressFile(
    const std::string& inputPath,
    const std::string& outputPath);

CompressionStats decompressFile(
    const std::string& inputPath,
    const std::string& outputPath);

#endif
