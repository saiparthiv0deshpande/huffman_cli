cmake_minimum_required(VERSION 3.16)

project(huffman_cli VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(huffman_core
    src/HuffmanTree.cpp
    src/BitWriter.cpp
    src/BitReader.cpp
    src/FileFormat.cpp
    src/Compressor.cpp
)

target_include_directories(huffman_core
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

add_executable(huff
    src/main.cpp
)

target_link_libraries(huff PRIVATE huffman_core)

enable_testing()

add_executable(huffman_tests
    tests/test_compression.cpp
)

target_link_libraries(huffman_tests PRIVATE huffman_core)

add_test(
    NAME huffman_compression_tests
    COMMAND huffman_tests
)
