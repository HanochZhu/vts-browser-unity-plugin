# bump version here
set(http_VERSION 1.8)

set(http_DEPENDS )

define_module(LIBRARY http=${http_VERSION}
  DEPENDS utility>=1.30 dbglog>=1.4 Boost CURL)

set(http_SOURCES
  http.hpp http.cpp
  contentgenerator.hpp contentgenerator.cpp
  contentfetcher.hpp
  resourcefetcher.hpp resourcefetcher.cpp
  ondemandclient.hpp ondemandclient.cpp

  detail/types.hpp
  detail/detail.hpp
  detail/acceptor.hpp
  detail/serverconnection.hpp

  detail/client.cpp

  detail/httpdate.hpp
)

if(WIN32)
  list(APPEND http_SOURCES
    detail/httpdate.windows.cpp
    )
else()
  list(APPEND http_SOURCES
    detail/httpdate.posix.cpp
    )
endif()

add_library(http STATIC ${http_SOURCES})
buildsys_library(http)
target_link_libraries(http ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(http ${MODULE_DEFINITIONS})

add_subdirectory(test)
