
if(NOT TARGET zlib)
    message(FATAL "zlib NOT FOUND")
endif()

if(NOT TARGET ZLIB::ZLIB)
    set(lib $<TARGET_LINKER_FILE:zlib>)
    get_target_property(inc zlib INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(ico zlib INTERFACE_COMPILE_OPTIONS)
    if(NOT ico)
      set(ico)
    endif()

    add_library(ZLIB::ZLIB INTERFACE IMPORTED GLOBAL)
    set_target_properties(ZLIB::ZLIB PROPERTIES
        INTERFACE_LINK_LIBRARIES "${lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${inc}"
        INTERFACE_COMPILE_OPTIONS "${ico}"
    )
    add_dependencies(ZLIB::ZLIB zlib)
endif()

set(Zlib_LIBRARIES ZLIB::ZLIB)
get_target_property(Zlib_INCLUDE_DIRS ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
set(Zlib_LIBRARY ${Zlib_LIBRARIES})
set(Zlib_INCLUDE_DIR ${Zlib_INCLUDE_DIRS})
set(Zlib_FOUND TRUE)

set(ZLIB_LIBRARIES ${Zlib_LIBRARIES})
set(ZLIB_INCLUDE_DIRS ${Zlib_INCLUDE_DIRS})
set(ZLIB_LIBRARY ${Zlib_LIBRARIES})
set(ZLIB_INCLUDE_DIR ${Zlib_INCLUDE_DIRS})
set(ZLIB_FOUND TRUE)

