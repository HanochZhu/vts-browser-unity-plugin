
message(STATUS "@@@ curl")

option(BUILD_CURL_EXE "Set to ON to build curl executable." OFF)
option(CURL_STATICLIB "Set to ON to build libcurl with static linking." ON)
option(HTTP_ONLY "disables all protocols except HTTP (This overrides all CURL_DISABLE_* options)" ON)
option(CMAKE_USE_LIBSSH2 "Use libSSH2" OFF)
if(WIN32)
    option(CMAKE_USE_WINSSL "enable Windows native SSL/TLS" ON)
elseif(APPLE)
    option(CMAKE_USE_SECTRANSP "enable Apple OS native SSL/TLS" ON)
else()
    option(CMAKE_USE_OPENSSL "use OpenSSL code" ON)
endif()
set(CURL_CA_PATH "none" CACHE STRING "ca path")

set(BUILD_TESTING OFF CACHE BOOL "BUILD_TESTING")
set(ENABLE_MANUAL OFF CACHE BOOL "ENABLE_MANUAL")
set(USE_MANUAL OFF CACHE BOOL "USE_MANUAL")

add_subdirectory(curl)

target_include_directories(libcurl INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/curl/include>)
target_compile_definitions(libcurl INTERFACE CURL_STATICLIB)
