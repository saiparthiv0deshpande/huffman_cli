#ifndef FILE_FORMAT_H
#define FILE_FORMAT_H

#include <array>
#include <cstdint>
#include <istream>
#include <ostream>

namespace HuffFile {

constexpr uint8_t VERSION = 1;
constexpr uint8_t MAGIC[4] = {'H', 'U', 'F', 'F'};

struct Header {
    uint64_t originalSize = 0;
    std::array<uint64_t, 256> frequencies{};
    uint16_t uniqueSymbols = 0;
    uint8_t paddingBits = 0;
};

void writeHeader(std::ostream& output, const Header& header);
Header readHeader(std::istream& input);

} // namespace HuffFile

#endif
