cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(vts-browser-build-wrapper)
include(externals/browser/browser/cmake/buildsys_initial_validation.cmake)

# make the project find our own provided libraries
# the libraries provided here should have higher priority than system libraries
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# make cmake search for additional programs
# the programs provided here should have higher priority than system programs
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/programs")

# MS Visual Studio
if(MSVC)
    # disable warning: class 'type' needs to have dll-interface to be used by clients of class 'type2'
    add_definitions(/wd4251 /wd4275)

    # disable warning: conversion from 'type' to 'type2', possible loss of data
    add_definitions(/wd4267 /wd4244 /wd4305)

    # disable warning: unary minus operator applied to unsigned type, result still unsigned
    add_definitions(/wd4146)

    # disable warning: deprecated declaration
    add_definitions(/wd4996)

    # enable multi process compilation
    add_definitions(/MP)

    add_definitions(/DWIN32_LEAN_AND_MEAN)
    add_definitions(/DNOMINMAX)
endif()

# all final product files should be placed in some sane locations
foreach(conf IN ITEMS ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
    string(TOUPPER ${conf} conf_upper)
    string(TOLOWER ${conf} conf_lower)
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY_${conf_upper})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${conf_upper} "${CMAKE_BINARY_DIR}/lib/${conf_lower}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${conf_upper} "${CMAKE_BINARY_DIR}/lib/${conf_lower}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${conf_upper} "${CMAKE_BINARY_DIR}/bin/${conf_lower}")
    endif()
endforeach(conf)

# 3rd party libraries
message(STATUS "@@@ including 3rd party libraries")
option(BUILD_TESTING "" OFF)
add_subdirectory(externals EXCLUDE_FROM_ALL)

# vts browser
message(STATUS "@@@ including vts browser")
add_subdirectory(externals/browser/browser)

# ide groups for 3rd party targets
foreach(name IN ITEMS genfiles jpeg-static GeographicLib_STATIC jsoncpp_lib jsoncpp_lib_static libcurl png_static proj SDL2-static simd simd-objs zlibstatic freetype harfbuzz hb-ft-interop
    example minigzip zlib genprebuilt gensym genvers symbol-check uninstall dmg wrjpgcom testclean dist rdjpgcom udmg jpegtran-static tjtest tarball turbojpeg-static djpeg-static tjunittest-static cjpeg-static tjbench-static jsimdcfg md5cmp SDL2main)
    if(TARGET ${name})
        buildsys_ide_groups(${name} deps)
    endif()
endforeach()
