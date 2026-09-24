#include "HuffmanTree.h"

#include <queue>
#include <utility>
#include <vector>

HuffmanTree::Node::Node(uint8_t byte, uint64_t frequency)
    : byte(byte), frequency(frequency) {}

HuffmanTree::Node::Node(uint64_t frequency,
                        std::shared_ptr<Node> left,
                        std::shared_ptr<Node> right)
    : byte(0), frequency(frequency),
      left(std::move(left)), right(std::move(right)) {}

bool HuffmanTree::Node::isLeaf() const {
    return left == nullptr && right == nullptr;
}

void HuffmanTree::build(
    const std::array<uint64_t, 256>& frequencies) {

    struct Compare {
        bool operator()(const std::shared_ptr<Node>& a,
                        const std::shared_ptr<Node>& b) const {
            return a->frequency > b->frequency;
        }
    };

    std::priority_queue<
        std::shared_ptr<Node>,
        std::vector<std::shared_ptr<Node>>,
        Compare
    > minHeap;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            minHeap.push(std::make_shared<Node>(
                static_cast<uint8_t>(i), frequencies[i]));
        }
    }

    root = nullptr;
    codes.clear();

    if (minHeap.empty()) {
        return;
    }

    while (minHeap.size() > 1) {
        auto left = minHeap.top();
        minHeap.pop();

        auto right = minHeap.top();
        minHeap.pop();

        minHeap.push(std::make_shared<Node>(
            left->frequency + right->frequency,
            left,
            right));
    }

    root = minHeap.top();
    generateCodes(root, "");
}

void HuffmanTree::generateCodes(
    const std::shared_ptr<Node>& node,
    const std::string& currentCode) {

    if (!node) {
        return;
    }

    if (node->isLeaf()) {
        codes[node->byte] =
            currentCode.empty() ? "0" : currentCode;
        return;
    }

    generateCodes(node->left, currentCode + "0");
    generateCodes(node->right, currentCode + "1");
}

const std::unordered_map<uint8_t, std::string>&
HuffmanTree::getCodes() const {
    return codes;
}
