
set(DOWNLOAD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")
set(USING_LOCAL_ARCHIVE FALSE)
set(ARCHIVE_PREFIX ${DOWNLOAD_PREFIX})

get_filename_component(BOOST_URL_FILENAME "${BOOST_URL}" NAME)
set(DOWNLOAD_PATH "${ARCHIVE_PREFIX}/${BOOST_URL_FILENAME}")
set(EXTRACT_PATH "${DOWNLOAD_PREFIX}/boost")

if(BOOST_SOURCE AND NOT IS_DIRECTORY "${BOOST_SOURCE}")
  message(WARNING "BOOST_SOURCE is not a directory")
  unset(BOOST_SOURCE CACHE)
endif()

if(BOOST_SOURCE AND NOT EXISTS "${BOOST_SOURCE}/boost/version.hpp")
  message(WARNING "Folder ${BOOST_SOURCE} does not contain boost/version.hpp")
  unset(BOOST_SOURCE CACHE)
endif()

if(BOOST_SOURCE AND NOT BOOST_LOCAL_SHA256 STREQUAL "${BOOST_SHA256}")
  message(WARNING "The current SHA does not match the expected SHA, removing the current version")
  file(REMOVE_RECURSE "${EXTRACT_PATH}")
  file(REMOVE "${DOWNLOAD_PATH}")
  unset(BOOST_SOURCE CACHE)
endif()

if(NOT BOOST_SOURCE)
  unset(BOOST_LOCAL_SHA256 CACHE)
endif()

function(boost_download_from_url url)
  if(NOT EXISTS "${DOWNLOAD_PATH}")
    file(REMOVE "${DOWNLOAD_PATH}.tmp") # Remove partially downloaded file

    message(STATUS "Downloading Boost from ${url}")
    file(DOWNLOAD ${url} "${DOWNLOAD_PATH}.tmp" STATUS download_status)

    list(GET download_status 0 download_status_code)
    list(GET download_status 1 download_status_message)
    if(download_status_code)
      message(WARNING "Download failed. Status: ${download_status_code} ${download_status_message}")
    else()
      file(SHA256 "${DOWNLOAD_PATH}.tmp" LOCAL_SHA256)
      if(NOT LOCAL_SHA256 STREQUAL "${BOOST_SHA256}")
        message(WARNING "Downloaded file does not match the expected SHA.\n Downloaded ${LOCAL_SHA256} != Expected ${BOOST_SHA256}")
      else()
        file(RENAME "${DOWNLOAD_PATH}.tmp" "${DOWNLOAD_PATH}") # commit the downloaded file
      endif()
    endif()
  endif()
endfunction()

if(NOT BOOST_SOURCE)
  message(STATUS "Downloading Boost")

  boost_download_from_url(${BOOST_URL})
  boost_download_from_url(${BOOST_URL_MIRROR})

  if(NOT EXISTS "${DOWNLOAD_PATH}")
    message(FATAL_ERROR "Downloads from all mirrors have failed.")
  endif()

  set(BOOST_LOCAL_SHA256 "${BOOST_SHA256}" CACHE STRING "Current SHA256 of Boost archive used" FORCE)
  mark_as_advanced(BOOST_LOCAL_SHA256)

  if(NOT IS_DIRECTORY "${EXTRACT_PATH}")
    message(STATUS "Extracting Boost archive")
    if(EXISTS "${EXTRACT_PATH}.tmp")
      file(REMOVE_RECURSE "${EXTRACT_PATH}.tmp")
    endif()
    file(MAKE_DIRECTORY "${EXTRACT_PATH}.tmp")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xfv ${DOWNLOAD_PATH}
      WORKING_DIRECTORY "${EXTRACT_PATH}.tmp"
      OUTPUT_QUIET
    )
    file(RENAME "${EXTRACT_PATH}.tmp" "${EXTRACT_PATH}")
  endif()

  message(STATUS "Searching the extracted folder.")
  file(GLOB download_boost_root "${EXTRACT_PATH}/boost_*")
  if(download_boost_root)
    set(BOOST_SOURCE "${download_boost_root}" CACHE STRING "Boost location" FORCE)
    mark_as_advanced(BOOST_SOURCE)
  endif()
endif()

# Detect Boost version
file(STRINGS "${BOOST_SOURCE}/boost/version.hpp" boost_version_raw REGEX "define BOOST_VERSION ")
string(REGEX MATCH "[0-9]+" boost_version_raw "${boost_version_raw}")
math(EXPR BOOST_VERSION_MAJOR "${boost_version_raw} / 100000")
math(EXPR BOOST_VERSION_MINOR "${boost_version_raw} / 100 % 1000")
math(EXPR BOOST_VERSION_PATCH "${boost_version_raw} % 100")
set(BOOST_VERSION "${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}.${BOOST_VERSION_PATCH}")

message(STATUS "Using Boost: ${BOOST_VERSION} ${BOOST_SOURCE}")

