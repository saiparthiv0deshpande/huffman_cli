cmake_minimum_required(VERSION 3.16)

project(huffman_cli VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(huff
    src/main.cpp
    src/HuffmanTree.cpp
    src/BitWriter.cpp
    src/BitReader.cpp
    src/FileFormat.cpp
    src/Compressor.cpp
)

target_include_directories(huff PRIVATE ${PROJECT_SOURCE_DIR}/include)
