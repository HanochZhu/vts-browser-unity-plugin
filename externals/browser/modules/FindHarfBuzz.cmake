
if(NOT TARGET harfbuzz)
    message(STATUS "hiding HarfBuzz (it is not yet defined)")
    return()
endif()

set(HarfBuzz_LIBRARIES harfbuzz)
get_target_property(HarfBuzz_INCLUDE_DIRS harfbuzz INTERFACE_INCLUDE_DIRECTORIES)
set(HarfBuzz_LIBRARY ${HarfBuzz_LIBRARIES})
set(HarfBuzz_INCLUDE_DIR ${HarfBuzz_INCLUDE_DIRS})
set(HarfBuzz_FOUND TRUE)

set(HARFBUZZ_LIBRARIES ${HarfBuzz_LIBRARIES})
set(HARFBUZZ_INCLUDE_DIRS ${HarfBuzz_INCLUDE_DIRS})
set(HARFBUZZ_LIBRARY ${HarfBuzz_LIBRARIES})
set(HARFBUZZ_INCLUDE_DIR ${HarfBuzz_INCLUDE_DIRS})
set(HARFBUZZ_FOUND TRUE)

