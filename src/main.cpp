#include "HuffmanTree.h"

#include <array>
#include <iostream>

int main() {
    std::array<uint64_t, 256> frequencies{};

    frequencies['A'] = 5;
    frequencies['B'] = 9;
    frequencies['C'] = 12;
    frequencies['D'] = 13;
    frequencies['E'] = 16;
    frequencies['F'] = 45;

    HuffmanTree tree;
    tree.build(frequencies);

    for (const auto& [byte, code] : tree.getCodes()) {
        std::cout << static_cast<char>(byte)
                  << " -> " << code << '\n';
    }

    return 0;
}
