
if(NOT TARGET png_static)
    message(FATAL "png_static NOT FOUND")
endif()

set(Png_LIBRARIES png_static)
get_target_property(Png_INCLUDE_DIRS png_static INTERFACE_INCLUDE_DIRECTORIES)
set(Png_LIBRARY ${Png_LIBRARIES})
set(Png_INCLUDE_DIR ${Png_INCLUDE_DIRS})
set(Png_FOUND TRUE)

set(PNG_LIBRARIES ${Png_LIBRARIES})
set(PNG_INCLUDE_DIRS ${Png_INCLUDE_DIRS})
set(PNG_LIBRARY ${Png_LIBRARIES})
set(PNG_INCLUDE_DIR ${Png_INCLUDE_DIRS})
set(PNG_FOUND TRUE)

