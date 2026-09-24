#ifndef BIT_READER_H
#define BIT_READER_H

#include <cstdint>
#include <istream>

class BitReader {
private:
    std::istream& input;
    uint8_t buffer = 0;
    uint8_t bitsRemaining = 0;
    uint64_t totalBits = 0;

public:
    explicit BitReader(std::istream& input);

    bool readBit(bool& bit);
    uint64_t bitsRead() const;
};

#endif
