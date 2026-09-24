#include "BitReader.h"
#include "BitWriter.h"
#include "Compressor.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::vector<uint8_t> readBytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    assert(input);

    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

void writeBytes(const fs::path& path, const std::vector<uint8_t>& data) {
    std::ofstream output(path, std::ios::binary);
    assert(output);

    for (uint8_t byte : data) {
        output.put(static_cast<char>(byte));
    }
}

void testBitWriterReader() {
    std::stringstream stream;
    BitWriter writer(stream);

    writer.writeBits("101100101");
    const uint8_t padding = writer.flush();

    assert(padding == 7);
    assert(writer.bitsWritten() == 9);

    BitReader reader(stream);
    bool bit = false;
    std::string bits;

    for (int i = 0; i < 9; ++i) {
        assert(reader.readBit(bit));
        bits += bit ? '1' : '0';
    }

    assert(bits == "101100101");
    assert(reader.bitsRead() == 9);
}

void testRoundTrip(
    const fs::path& directory,
    const std::string& name,
    const std::vector<uint8_t>& original) {

    const fs::path input = directory / (name + ".input");
    const fs::path compressed = directory / (name + ".huff");
    const fs::path restored = directory / (name + ".output");

    writeBytes(input, original);

    const CompressionStats encodeStats =
        compressFile(input.string(), compressed.string());

    const CompressionStats decodeStats =
        decompressFile(compressed.string(), restored.string());

    assert(encodeStats.originalSize == original.size());
    assert(decodeStats.originalSize == original.size());
    assert(readBytes(restored) == original);

    fs::remove(input);
    fs::remove(compressed);
    fs::remove(restored);
}

void testEmptyFile(const fs::path& directory) {
    testRoundTrip(directory, "empty", {});
}

void testSingleSymbol(const fs::path& directory) {
    testRoundTrip(directory, "single_symbol",
                  std::vector<uint8_t>(1000, static_cast<uint8_t>('A')));
}

void testTextFile(const fs::path& directory) {
    const std::string text =
        "Huffman coding is a lossless compression algorithm. "
        "This test contains repeated text to make compression useful.\n";

    testRoundTrip(
        directory,
        "text",
        std::vector<uint8_t>(text.begin(), text.end()));
}

void testAllByteValues(const fs::path& directory) {
    std::vector<uint8_t> data;

    for (int repetition = 0; repetition < 4; ++repetition) {
        for (int value = 0; value < 256; ++value) {
            data.push_back(static_cast<uint8_t>(value));
        }
    }

    testRoundTrip(directory, "all_bytes", data);
}

void testRandomData(const fs::path& directory) {
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(0, 255);

    std::vector<uint8_t> data(4096);

    for (uint8_t& byte : data) {
        byte = static_cast<uint8_t>(distribution(generator));
    }

    testRoundTrip(directory, "random", data);
}

} // namespace

int main() {
    const fs::path directory =
        fs::temp_directory_path() / "huffman_cli_tests";

    fs::create_directories(directory);

    testBitWriterReader();
    testEmptyFile(directory);
    testSingleSymbol(directory);
    testTextFile(directory);
    testAllByteValues(directory);
    testRandomData(directory);

    fs::remove_all(directory);

    return 0;
}
