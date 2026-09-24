# Huffman CLI

A C++17 command-line utility for **lossless file compression and decompression using Huffman coding**.

The project implements frequency analysis, Huffman tree construction, bit-level I/O, a custom binary file format, integrity validation, automated tests, and performance benchmarks.

## Features
- Huffman coding with a min-heap priority queue
- Lossless compression and decompression
- Supports arbitrary binary files
- Bit-level input/output
- Custom versioned `.huff` binary format
- Tree-based decoding
- Corruption and truncation detection
- `info` command for inspecting compressed files
- Automated round-trip and corruption tests
- C++17 + CMake
- Benchmark executable

## Commands
```text
huff encode <input> <output>
huff decode <input> <output>
huff info <file>
```

## Build
Requirements: C++17 compiler and CMake 3.16+.

```bash
cmake -S . -B build
cmake --build build
```

## Testing
```bash
ctest --test-dir build
```

The tests cover bit I/O, empty files, single-symbol files, text, all 256 byte values, random binary data, corrupted headers, truncated payloads, and byte-for-byte encode/decode round trips.

## Benchmarking
```bash
./build/huffman_benchmark
```

The benchmark measures original/compressed size, compression percentage, encoding/decoding time, and throughput for 1 MB, 5 MB, and 10 MB repeated-text datasets.

## Architecture
```text
CLI
  |
Compressor
  |-- Frequency Table --> Huffman Tree / Min-Heap --> Codes
  |                                                |
  |                                             BitWriter
  |                                                |
  +------------------------------------------> .huff
                                                   |
                                                BitReader
                                                   |
                                             Huffman Tree
                                                   |
                                             Original File
```

## File Format
The current format is version 2:

```text
Magic:            4 bytes (HUFF)
Version:          1 byte
Original size:    u64
Encoded bits:     u64
Unique symbols:   u16
Symbol/frequency: repeated for each symbol
Payload:          compressed bit stream
```

Integer metadata is stored in little-endian form. The decoder validates the header, frequency total, payload size, encoded bit count, and padding.

## Complexity
For `n` input bytes and at most `k = 256` distinct byte values:

- Frequency analysis: O(n)
- Huffman tree construction: O(k log k)
- Code generation: O(k)
- Encoding: O(nL), where L is average code length
- Decoding: O(nL)
- Tree/code memory: O(k)

## Project Structure
```text
huffman_cli/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── BitReader.h
│   ├── BitWriter.h
│   ├── Compressor.h
│   ├── FileFormat.h
│   └── HuffmanTree.h
├── src/
│   ├── BitReader.cpp
│   ├── BitWriter.cpp
│   ├── Compressor.cpp
│   ├── FileFormat.cpp
│   ├── HuffmanTree.cpp
│   └── main.cpp
├── tests/
│   └── test_compression.cpp
└── benchmarks/
    └── benchmark.cpp
```

## Example Workflow
```bash
cmake -S . -B build
cmake --build build
./build/huff encode sample.txt sample.huff
./build/huff info sample.huff
./build/huff decode sample.huff restored.txt
diff sample.txt restored.txt
ctest --test-dir build
./build/huffman_benchmark
```

## Technologies
C++17, STL, CMake, binary file I/O, priority queues, hash maps, bit manipulation, and recursive tree traversal.

## Future Improvements
- Canonical Huffman codes
- Parallel frequency analysis
- Streaming library API
- More benchmark datasets
- Comparison with other compression algorithms