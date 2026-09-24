#include "BitWriter.h"

#include <stdexcept>

BitWriter::BitWriter(std::ostream& output)
    : output(output) {}

void BitWriter::writeBit(bool bit) {
    buffer = static_cast<uint8_t>((buffer << 1) | (bit ? 1 : 0));
    ++bitCount;
    ++totalBits;

    if (bitCount == 8) {
        output.put(static_cast<char>(buffer));
        if (!output) {
            throw std::runtime_error("Failed to write bitstream");
        }
        buffer = 0;
        bitCount = 0;
    }
}

void BitWriter::writeBits(const std::string& bits) {
    for (char bit : bits) {
        if (bit == '0') {
            writeBit(false);
        } else if (bit == '1') {
            writeBit(true);
        } else {
            throw std::invalid_argument("Bit string must contain only 0 or 1");
        }
    }
}

uint8_t BitWriter::flush() {
    if (bitCount == 0) {
        return 0;
    }

    const uint8_t paddingBits =
        static_cast<uint8_t>(8 - bitCount);

    buffer = static_cast<uint8_t>(buffer << paddingBits);

    output.put(static_cast<char>(buffer));
    if (!output) {
        throw std::runtime_error("Failed to flush bitstream");
    }

    buffer = 0;
    bitCount = 0;

    return paddingBits;
}

uint64_t BitWriter::bitsWritten() const {
    return totalBits;
}
