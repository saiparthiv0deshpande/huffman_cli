#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

class HuffmanTree {
private:
    struct Node {
        uint8_t byte;
        uint64_t frequency;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;

        Node(uint8_t byte, uint64_t frequency);
        Node(uint64_t frequency,
             std::shared_ptr<Node> left,
             std::shared_ptr<Node> right);

        bool isLeaf() const;
    };

    std::shared_ptr<Node> root;
    std::unordered_map<uint8_t, std::string> codes;

    void generateCodes(const std::shared_ptr<Node>& node,
                       const std::string& currentCode);

public:
    void build(const std::array<uint64_t, 256>& frequencies);

    const std::unordered_map<uint8_t, std::string>& getCodes() const;
};

#endif
