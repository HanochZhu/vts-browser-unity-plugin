cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

message(STATUS "Using WASM dummy")
add_library(jpeg-static STATIC wasm.cpp)
target_include_directories(jpeg-static INTERFACE .)
target_compile_options(jpeg-static PUBLIC "SHELL:-s USE_LIBJPEG=1")
target_link_options(jpeg-static PUBLIC "SHELL:-s USE_LIBJPEG=1")
