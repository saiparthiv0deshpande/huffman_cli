#include "BitReader.h"

#include <stdexcept>

BitReader::BitReader(std::istream& input)
    : input(input) {
    if (!input.good()) {
        throw std::runtime_error("Input stream is not ready");
    }
}

bool BitReader::readBit(bool& bit) {
    if (bitsRemaining == 0) {
        char byte = 0;

        if (!input.get(byte)) {
            return false;
        }

        buffer = static_cast<uint8_t>(
            static_cast<unsigned char>(byte));
        bitsRemaining = 8;
    }

    // Read from most-significant to least-significant bit.
    bit = (buffer & 0x80u) != 0;

    buffer = static_cast<uint8_t>(buffer << 1);
    --bitsRemaining;
    ++totalBits;

    return true;
}

uint64_t BitReader::bitsRead() const {
    return totalBits;
}
