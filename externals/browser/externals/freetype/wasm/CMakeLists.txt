cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

message(STATUS "Using WASM dummy")
add_library(freetype STATIC wasm.cpp)
target_include_directories(freetype INTERFACE .)
target_compile_options(freetype PUBLIC "SHELL:-s USE_FREETYPE=1")
target_link_options(freetype PUBLIC "SHELL:-s USE_FREETYPE=1")
