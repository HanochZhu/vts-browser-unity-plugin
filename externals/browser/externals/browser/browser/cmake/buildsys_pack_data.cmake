
function(buildsys_pack_data NAME)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SRC_LIST)
set(GEN_CODE)
set(GEN_CODE "${GEN_CODE}\#include <string>\n")
set(GEN_CODE "${GEN_CODE}\#include <stdint.h>\n")
set(GEN_CODE "${GEN_CODE}namespace vts { namespace detail { void addInternalMemoryData(const std::string &name, const unsigned char *data, uint32_t size)\; } }\n")
set(GEN_CODE "${GEN_CODE}using namespace vts::detail\;\n")
set(GEN_CODE "${GEN_CODE}void ${NAME}()\n{\n")
foreach(path ${DATA_LIST})
    set_source_files_properties(${path} PROPERTIES HEADER_FILE_ONLY TRUE) # do not compile that file
    string(REPLACE "/" "_" name ${path})
    string(REPLACE "." "_" name ${name})
    string(REPLACE "-" "_" name ${name})
    file_to_cpp("" ${name} ${path})
    list(APPEND SRC_LIST ${CMAKE_CURRENT_BINARY_DIR}/${path}.hpp)
    list(APPEND SRC_LIST ${CMAKE_CURRENT_BINARY_DIR}/${path}.cpp)
    set(GEN_CODE "\#include \"${CMAKE_CURRENT_BINARY_DIR}/${path}.hpp\"\n${GEN_CODE}")
    set(GEN_CODE "${GEN_CODE}addInternalMemoryData(\"${path}\", ${name}, sizeof(${name}))\;\n")
endforeach()
set(GEN_CODE "${GEN_CODE}}\n\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/data_map.cpp ${GEN_CODE})
list(APPEND SRC_LIST ${CMAKE_CURRENT_BINARY_DIR}/data_map.cpp)
add_custom_target(${NAME}_data SOURCES ${DATA_LIST})
buildsys_ide_groups(${NAME}_data data)
add_library(${NAME} STATIC ${SRC_LIST})
buildsys_ide_groups(${NAME} data)
add_dependencies(${NAME} ${NAME}_data)
endfunction(buildsys_pack_data)
