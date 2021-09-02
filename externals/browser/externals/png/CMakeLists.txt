
message(STATUS "@@@ png")

if(BUILDSYS_WASM)
    add_subdirectory(wasm)
    return()
endif()

if(BUILDSYS_IOS)
    set(PNG_HARDWARE_OPTIMIZATIONS "on" CACHE STRING "Enable Hardware Optimizations" FORCE)
    set(PNG_ARM_NEON "on" CACHE STRING "Enable ARM NEON optimizations" FORCE)
    set(ANDROID ON) # forces the png to use pregenerated config file
endif()

option(PNG_SHARED "Build shared lib" OFF)
option(PNG_TESTS  "Build libpng tests" OFF)
set(SKIP_INSTALL_ALL ON)

add_subdirectory(png)

if(BUILDSYS_IOS)
    target_sources(png_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/png/arm/arm_init.c")
    target_sources(png_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/png/arm/filter_neon_intrinsics.c")
    target_sources(png_static PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/png/arm/palette_neon_intrinsics.c")
    target_compile_definitions(png_static PRIVATE -DPNG_ARM_NEON_OPT=2)
endif()

target_include_directories(png_static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/png")
target_include_directories(png_static INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/png")

