cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

message(STATUS "Using WASM dummy")
add_library(png_static STATIC wasm.cpp)
target_include_directories(png_static INTERFACE .)
target_compile_options(png_static PUBLIC "SHELL:-s USE_LIBPNG=1")
target_link_options(png_static PUBLIC "SHELL:-s USE_LIBPNG=1")
