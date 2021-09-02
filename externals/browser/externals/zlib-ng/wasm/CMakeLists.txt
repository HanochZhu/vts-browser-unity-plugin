cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

message(STATUS "Using WASM dummy")
add_library(zlib STATIC wasm.cpp)
target_include_directories(zlib INTERFACE .)
target_compile_options(zlib PUBLIC "SHELL:-s USE_ZLIB=1")
target_link_options(zlib PUBLIC "SHELL:-s USE_ZLIB=1")
