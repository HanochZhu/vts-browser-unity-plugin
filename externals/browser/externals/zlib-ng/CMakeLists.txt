
message(STATUS "@@@ zlib (zlib-ng)")

if(BUILDSYS_WASM)
    add_subdirectory(wasm)
    return()
endif()

option(ZLIB_COMPAT "Compile with zlib compatible API" ON)
option(ZLIB_ENABLE_TESTS "Build test binaries" OFF)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "BUILD_SHARED_LIBS")
set(SKIP_INSTALL_ALL ON)
unset(ENV{CI})


# temporary workaround: https://github.com/zlib-ng/zlib-ng/issues/633
set(WITH_AVX2 OFF CACHE BOOL "" FORCE)


add_subdirectory(zlib-ng)

target_include_directories(zlib INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/zlib-ng")
target_include_directories(zlib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/zlib-ng")

if(APPLE)
    target_compile_options(zlib PRIVATE -Wno-implicit-function-declaration)
endif()
