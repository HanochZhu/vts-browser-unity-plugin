
message(STATUS "@@@ freetype")

if(BUILDSYS_WASM)
    add_subdirectory(wasm)
    return()
endif()

option(FT_WITH_ZLIB "Use system zlib instead of internal library." ON)
set(BUILD_SHARED_LIBS OFF)
set(SKIP_INSTALL_ALL ON)

if(IOS_PLATFORM STREQUAL "iphoneos")
    set(IOS_PLATFORM "OS")
endif()

add_subdirectory(freetype2)
