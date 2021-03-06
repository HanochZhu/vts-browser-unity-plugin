macro(enable_OpenMP_impl)
  if((${CMAKE_CXX_COMPILER_ID} MATCHES GNU) OR MSVC)
    find_package(OpenMP)
  endif()

  if(OPENMP_FOUND)
    message(STATUS "Found OpenMP.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${OpenMP_Fortran_FLAGS}")

    if(NOT OpenMP_FOUND)
      # for compatibility with older cmake version
      set(OpenMP_FOUND TRUE)
    endif()
  else()
    message(STATUS "OpenMP not available. Your program will lack parallelism.")
  endif()
endmacro()

macro(enable_OpenMP)
  set(enable TRUE)

  if(BUILDSYS_DISABLE_OPENMP)
    set(enable FALSE)
  elseif(INTERNAL_BUILDSYS_DISABLE_OPENMP)
    set(enable FALSE)
  endif()

  if(enable)
    enable_OpenMP_impl()
  else()
    message(STATUS "Disabling OpenMP support because of BUILDSYS_DISABLE_OPENMP.")
  endif()
endmacro()
