
message(STATUS "@@@ proj")

option(PROJ_TESTS "Enable build of collection of PROJ tests" OFF)
option(PROJ4_TESTS "Enable build of collection of PROJ tests" OFF)
option(BUILD_CCT     "Build cct (coordinate conversion and transformation tool)" OFF)
option(BUILD_CS2CS   "Build cs2cs (coordinate systems to coordinate systems translation tool)" OFF)
option(BUILD_GEOD    "Build geod (computation of geodesic lines)" OFF)
option(BUILD_GIE     "Build gie (geospatial integrity investigation environment - a PROJ.4 test tool)" OFF)
option(BUILD_NAD2BIN "Build nad2bin (format conversion tool)" OFF)
option(BUILD_PROJ    "Build proj (cartographic projection tool : latlong <-> projected coordinates)" OFF)
option(BUILD_LIBPROJ_SHARED "Build libproj library shared." OFF)


# add dummy targets so that unguarded set_target_properties does not fail
add_library(cs2cs UNKNOWN IMPORTED)
add_library(binproj UNKNOWN IMPORTED)
add_library(geod UNKNOWN IMPORTED)
add_library(nad2bin UNKNOWN IMPORTED)


add_subdirectory(proj)

target_include_directories(proj INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/proj/src>)

