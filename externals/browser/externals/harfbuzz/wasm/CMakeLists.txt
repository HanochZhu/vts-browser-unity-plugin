cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

message(STATUS "Using WASM dummy")
add_library(harfbuzz STATIC wasm.cpp)
target_include_directories(harfbuzz INTERFACE .)
target_compile_options(harfbuzz PUBLIC "SHELL:-s USE_HARFBUZZ=1")
target_link_options(harfbuzz PUBLIC "SHELL:-s USE_HARFBUZZ=1")
