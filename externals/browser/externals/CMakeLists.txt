
cmake_policy(SET CMP0042 NEW) # use rpath
set(CMAKE_MACOSX_BUNDLE FALSE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)

# disable warnings
if(MSVC)
    string(REPLACE "/W3" "/W0" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    string(REPLACE "/W3" "/W0" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

# disable warnings that some files produced no symbols
if(MSVC)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ignore:4221")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -ignore:4221")
endif()

# compile all 3rd party dependencies with optimizations even in debug build
if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O2")
    # /RTC and /O2 are incompatible
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
        )
        string(REGEX REPLACE "/RTC[^ ]*" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
else()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")
endif()

add_subdirectory(zlib-ng)
add_subdirectory(jsoncpp)
add_subdirectory(png)
add_subdirectory(jpeg)
add_subdirectory(eigen)
add_subdirectory(boost)
add_subdirectory(proj)
add_subdirectory(geographiclib)
add_subdirectory(freetype)
add_subdirectory(harfbuzz)

if(NOT BUILDSYS_EMBEDDED)
    add_subdirectory(sdl)
    add_subdirectory(curl)
endif()
