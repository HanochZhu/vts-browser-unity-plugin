
if(TARGET jsoncpp_lib_static)
    set(jsoncpp_lib_target jsoncpp_lib_static)
else()
    set(jsoncpp_lib_target jsoncpp_lib)
endif()

set(JsonCPP_LIBRARIES ${jsoncpp_lib_target})
get_target_property(JsonCPP_INCLUDE_DIRS ${jsoncpp_lib_target} INTERFACE_INCLUDE_DIRECTORIES)
set(JsonCPP_LIBRARY ${JsonCPP_LIBRARIES})
set(JsonCPP_INCLUDE_DIR ${JsonCPP_INCLUDE_DIRS})
set(JsonCPP_FOUND TRUE)

set(JSONCPP_LIBRARIES ${JsonCPP_LIBRARIES})
set(JSONCPP_INCLUDE_DIRS ${JsonCPP_INCLUDE_DIRS})
set(JSONCPP_LIBRARY ${JsonCPP_LIBRARIES})
set(JSONCPP_INCLUDE_DIR ${JsonCPP_INCLUDE_DIRS})
set(JSONCPP_FOUND TRUE)


