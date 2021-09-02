cmake_minimum_required(VERSION 3.10)
project(Boost-CMake)

if(NOT BOOST_URL)
  set(BOOST_URL "https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.bz2")
endif()

if(NOT BOOST_URL_MIRROR)
  set(BOOST_URL_MIRROR "https://sourceforge.net/projects/boost/files/boost/1.72.0/boost_1_72_0.tar.bz2")
endif()

if(NOT BOOST_SHA256)
  set(BOOST_SHA256 "59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722")
endif()

if(NOT BOOST_LIBS)
  set(BOOST_LIBS
    header
    atomic
    chrono
    container
    context
    #coroutine
    date_time
    exception
    #fiber
    filesystem
    #graph
    iostreams
    #locale
    #log
    math
    #mpi
    #graph_parallel # depends on mpi, so needs to be put after it
    program_options
    #python # complex module
    #random
    regex
    serialization
    #signals
    system
    test
    thread
    timer
    #type_erasure
    #wave
  )
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

include(DownloadBoost)
include(PlatformDetect)
include(AddBoostLib)
include(AddBoostTest)

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

if(USE_ANDROID)
  # CMake 3.7.1 doesn't define the target triple for the ASM language,
  # resulting in all files compiled for the host architecture
  set(CMAKE_ASM_COMPILER_TARGET "${CMAKE_CXX_COMPILER_TARGET}")
endif()

foreach(lib ${BOOST_LIBS})
  include("libs/${lib}.cmake")
endforeach()

# Compilation options required by all platforms
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS $<$<CONFIG:Release>:BOOST_DISABLE_ASSERT>)
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_SYSTEM_NO_DEPRECATED)
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_THREAD_VERSION=4)
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_THREAD_USES_CHRONO)
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_THREAD_PROVIDES_EXECUTORS)

# make boost scoped_lock available as a nested class of mutex
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_THREAD_PROVIDES_NESTED_LOCKS)
# configure boost for manual linking libraries
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_ALL_NO_LIB)
# suppress boost compiler version check warnings
set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

if(UNIX)
  set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -Wno-unused-local-typedefs)
endif()

if(USE_ANDROID)
  # Android doesn't support thread local storage through compiler intrinsics
  set_property(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_ASIO_DISABLE_THREAD_KEYWORD_EXTENSION)
endif()
