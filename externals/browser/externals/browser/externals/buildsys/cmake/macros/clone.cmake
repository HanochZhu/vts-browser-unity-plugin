macro(buildsys_clone_target src-target dst-target)
  set(mapping_ ${ARGV2})
  if(NOT TARGET ${dst-target})
    message(STATUS "Cloning target ${src-target} into ${dst-target}.")

    get_target_property(src-SOURCES ${src-target} SOURCES)
    get_target_property(src-HOME ${src-target} BUILDSYS_HOME_DIR)
    unset(dst-SOURCES)
    foreach(source ${src-SOURCES})
      string(REGEX MATCH "^/" matched ${source})
      if(matched)
        list(APPEND dst-SOURCES ${source})
      else()
        list(APPEND dst-SOURCES ${src-HOME}/${source})
      endif()
    endforeach()

    get_target_property(LINK_LIBRARIES ${src-target} LINK_LIBRARIES)
    get_target_property(INCLUDE_DIRECTORIES ${src-target} INCLUDE_DIRECTORIES)
    get_target_property(COMPILE_DEFINITIONS ${src-target} COMPILE_DEFINITIONS)

    if(mapping_)
      if(LINK_LIBRARIES)
        unset(tmp)
        foreach(lib ${LINK_LIBRARIES})
          dict(GET ${mapping_} ${lib} mapped)
          if(mapped)
            list(APPEND tmp ${mapped})
          else()
            list(APPEND tmp ${lib})
          endif()
        endforeach()
        set(LINK_LIBRARIES ${tmp})
      endif()

      # remember mapping
      dict(SET ${ARGV2} ${src-target} ${dst-target})
    endif()

    get_target_property(TYPE ${src-target} TYPE)
    if(TYPE STREQUAL "STATIC_LIBRARY")
      add_library(${dst-target} STATIC ${dst-SOURCES})
      buildsys_library(${dst-target})
      if(LINK_LIBRARIES)
        target_link_libraries(${dst-target} ${LINK_LIBRARIES})
      endif()
      if(INCLUDE_DIRECTORIES)
        target_include_directories(${dst-target} PUBLIC ${INCLUDE_DIRECTORIES})
      endif()
      if(COMPILE_DEFINITIONS)
          target_compile_definitions(${dst-target} ${COMPILE_DEFINITIONS})
      endif()

      # make a module from target
      set(MODULE_${dst-target}_FOUND TRUE)
      set(MODULE_${dst-target}_FOUND TRUE PARENT_SCOPE)
    else()
      message(FATAL_ERROR "Don't know how to duplicate target of type <${TYPE}>.")
    endif()

    add_dependencies(${dst-target} ${src-target})
  elseif(mapping_)
    dict(SET ${ARGV2} ${src-target} ${dst-target})
  endif()
endmacro()