#include "FileFormat.h"

#include <limits>
#include <stdexcept>

namespace {

void writeU16(std::ostream& out, uint16_t value) {
    out.put(static_cast<char>(value & 0xff));
    out.put(static_cast<char>((value >> 8) & 0xff));
}

void writeU64(std::ostream& out, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        out.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

uint16_t readU16(std::istream& in) {
    uint16_t value = 0;
    for (int i = 0; i < 2; ++i) {
        const int c = in.get();
        if (c == EOF) {
            throw std::runtime_error("Truncated Huffman header");
        }
        value |= static_cast<uint16_t>(
            static_cast<uint16_t>(c) << (i * 8));
    }
    return value;
}

uint64_t readU64(std::istream& in) {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        const int c = in.get();
        if (c == EOF) {
            throw std::runtime_error("Truncated Huffman header");
        }
        value |= static_cast<uint64_t>(
            static_cast<uint64_t>(static_cast<uint8_t>(c)) << (i * 8));
    }
    return value;
}

} // namespace

namespace HuffFile {

void writeHeader(std::ostream& output, const Header& header) {
    for (uint8_t byte : MAGIC) {
        output.put(static_cast<char>(byte));
    }

    output.put(static_cast<char>(VERSION));
    writeU64(output, header.originalSize);
    writeU64(output, header.encodedBits);

    uint16_t count = 0;
    for (uint64_t frequency : header.frequencies) {
        if (frequency > 0) {
            ++count;
        }
    }

    writeU16(output, count);

    for (uint16_t byte = 0; byte < 256; ++byte) {
        if (header.frequencies[byte] > 0) {
            output.put(static_cast<char>(byte));
            writeU64(output, header.frequencies[byte]);
        }
    }

    if (!output) {
        throw std::runtime_error("Failed to write Huffman header");
    }
}

Header readHeader(std::istream& input) {
    Header header{};

    for (uint8_t expected : MAGIC) {
        const int c = input.get();
        if (c == EOF || static_cast<uint8_t>(c) != expected) {
            throw std::runtime_error("Invalid Huffman file");
        }
    }

    const int version = input.get();
    if (version == EOF || static_cast<uint8_t>(version) != VERSION) {
        throw std::runtime_error("Unsupported Huffman file version");
    }

    header.originalSize = readU64(input);
    header.encodedBits = readU64(input);
    header.uniqueSymbols = readU16(input);

    if (header.uniqueSymbols > 256) {
        throw std::runtime_error("Invalid symbol count in Huffman header");
    }

    uint64_t frequencySum = 0;

    for (uint16_t i = 0; i < header.uniqueSymbols; ++i) {
        const int byte = input.get();
        if (byte == EOF) {
            throw std::runtime_error("Truncated Huffman header");
        }

        const uint8_t symbol = static_cast<uint8_t>(byte);

        if (header.frequencies[symbol] != 0) {
            throw std::runtime_error("Duplicate symbol in Huffman header");
        }

        const uint64_t frequency = readU64(input);

        if (frequency == 0) {
            throw std::runtime_error("Invalid zero frequency");
        }

        if (frequencySum > std::numeric_limits<uint64_t>::max() - frequency) {
            throw std::runtime_error("Frequency total overflow");
        }

        header.frequencies[symbol] = frequency;
        frequencySum += frequency;
    }

    if (frequencySum != header.originalSize) {
        throw std::runtime_error("Frequency total does not match original size");
    }

    if (header.originalSize == 0) {
        if (header.uniqueSymbols != 0 || header.encodedBits != 0) {
            throw std::runtime_error("Invalid empty-file header");
        }
    } else {
        if (header.uniqueSymbols == 0 || header.encodedBits == 0) {
            throw std::runtime_error("Invalid non-empty Huffman header");
        }
    }

    return header;
}

} // namespace HuffFile
