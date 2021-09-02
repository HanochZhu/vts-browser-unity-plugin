
message(STATUS "@@@ jsoncpp")

OPTION(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" OFF)
OPTION(JSONCPP_WITH_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" OFF)
OPTION(JSONCPP_WITH_PKGCONFIG_SUPPORT "Generate and install .pc files" OFF)
OPTION(JSONCPP_WITH_CMAKE_PACKAGE "Generate and install cmake package files" OFF)
option(BUILD_SHARED_LIBS "Build jsoncpp_lib as a shared library." OFF)

add_subdirectory(jsoncpp)

if(TARGET jsoncpp_lib_static)
    set(jsoncpp_lib_target jsoncpp_lib_static)
else()
    set(jsoncpp_lib_target jsoncpp_lib)
endif()

foreach(conf IN ITEMS ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
    string(TOUPPER ${conf} conf_upper)
    set_target_properties(${jsoncpp_lib_target} PROPERTIES OUTPUT_NAME_${conf_upper} jsoncpp_3rd_party)
endforeach()


# prevent the library from overriding my cxx standard flags
set_target_properties(${jsoncpp_lib_target} PROPERTIES COMPILE_FEATURES "")
set_target_properties(${jsoncpp_lib_target} PROPERTIES INTERFACE_COMPILE_FEATURES "")


