#ifndef BIT_WRITER_H
#define BIT_WRITER_H

#include <cstdint>
#include <ostream>
#include <string>

class BitWriter {
private:
    std::ostream& output;
    uint8_t buffer = 0;
    uint8_t bitCount = 0;
    uint64_t totalBits = 0;

public:
    explicit BitWriter(std::ostream& output);

    void writeBit(bool bit);
    void writeBits(const std::string& bits);
    uint8_t flush();
    uint64_t bitsWritten() const;
};

#endif
