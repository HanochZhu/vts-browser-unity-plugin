# test program
define_module(BINARY http-selftest
  DEPENDS
  http service
  )

set(http-selftest_SOURCES
  main.cpp
  )

add_executable(http-selftest ${http-selftest_SOURCES})
target_link_libraries(http-selftest ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(http-selftest ${MODULE_DEFINITIONS})
buildsys_binary(http-selftest)
set_target_version(http-selftest "test")
