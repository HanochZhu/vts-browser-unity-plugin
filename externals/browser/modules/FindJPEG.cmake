
if(NOT TARGET jpeg-static)
    message(FATAL "jpeg-static NOT FOUND")
endif()

set(Jpeg_LIBRARIES jpeg-static)
get_target_property(Jpeg_INCLUDE_DIRS jpeg-static INTERFACE_INCLUDE_DIRECTORIES)
set(Jpeg_LIBRARY ${Jpeg_LIBRARIES})
set(Jpeg_INCLUDE_DIR ${Jpeg_INCLUDE_DIRS})
set(Jpeg_FOUND TRUE)

set(JPEG_LIBRARIES ${Jpeg_LIBRARIES})
set(JPEG_INCLUDE_DIRS ${Jpeg_INCLUDE_DIRS})
set(JPEG_LIBRARY ${Jpeg_LIBRARIES})
set(JPEG_INCLUDE_DIR ${Jpeg_INCLUDE_DIRS})
set(JPEG_FOUND TRUE)

