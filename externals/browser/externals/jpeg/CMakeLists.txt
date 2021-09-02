
message(STATUS "@@@ jpeg")

if(BUILDSYS_WASM)
    add_subdirectory(wasm)
    return()
endif()

option(ENABLE_SHARED "Build shared libraries" FALSE)

if(APPLE)
  option(WITH_SIMD "Include SIMD extensions, if available for this platform" FALSE)
endif()

if(MSVC)
    option(WITH_CRT_DLL "/MT vs /MD" TRUE)
endif()

if(NOT CMAKE_INSTALL_DOCDIR)
    set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_BINDIR})
endif()

add_subdirectory(jpeg)

target_include_directories(jpeg-static INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/jpeg")
target_include_directories(jpeg-static INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/jpeg")

