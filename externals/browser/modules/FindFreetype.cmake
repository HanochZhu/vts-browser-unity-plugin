
if(NOT TARGET freetype)
    message(FATAL "freetype NOT FOUND")
endif()

set(Freetype_LIBRARIES freetype)
get_target_property(Freetype_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)
set(Freetype_LIBRARY ${Freetype_LIBRARIES})
set(Freetype_INCLUDE_DIR ${Freetype_INCLUDE_DIRS})
set(Freetype_FOUND TRUE)

set(FREETYPE_LIBRARIES ${Freetype_LIBRARIES})
set(FREETYPE_INCLUDE_DIRS ${Freetype_INCLUDE_DIRS})
set(FREETYPE_LIBRARY ${Freetype_LIBRARIES})
set(FREETYPE_INCLUDE_DIR ${Freetype_INCLUDE_DIRS})
set(FREETYPE_FOUND TRUE)

