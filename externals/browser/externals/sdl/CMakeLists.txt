
message(STATUS "@@@ sdl")

set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build a static version of the library" FORCE)

add_subdirectory(sdl)

function(directory_link target name)
    if(NOT EXISTS ${name})
        if(WIN32)
            file(TO_NATIVE_PATH ${target} target)
            file(TO_NATIVE_PATH ${name} name)
            execute_process(COMMAND cmd /C mklink /D "${name}" "${target}")
        else()
            execute_process(COMMAND ln -s "${target}" "${name}")
        endif()
    endif()
endfunction(directory_link)
directory_link("${CMAKE_CURRENT_SOURCE_DIR}/sdl/include" "${CMAKE_CURRENT_BINARY_DIR}/SDL2")

target_include_directories(SDL2-static INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

add_library(SDL2 ALIAS SDL2-static)
