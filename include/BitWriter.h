#ifndef BIT_WRITER_H
#define BIT_WRITER_H

#include <cstdint>
#include <fstream>
#include <string>

class BitWriter {
private:
    std::ofstream& output;
    uint8_t buffer = 0;
    uint8_t bitCount = 0;
    uint64_t totalBits = 0;

public:
    explicit BitWriter(std::ofstream& output);

    void writeBit(bool bit);
    void writeBits(const std::string& bits);

    // Flushes the final partial byte. Returns the number of padding bits.
    uint8_t flush();

    uint64_t bitsWritten() const;
};

#endif
