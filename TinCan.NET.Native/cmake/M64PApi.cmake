if (REFRESH_MUPEN_API OR NOT MUPEN_API_DOWNLOADED)
  set(MUPEN_API_DOWNLOADED TRUE CACHE INTERNAL "True if Mupen has been downloaded.")
  unset(REFRESH_MUPEN_API CACHE)
  # Download rerecording (in case of API differences)
  file(DOWNLOAD "https://github.com/Mupen64-Rewrite/mupen64plus-core-rr/archive/master.tar.gz"
    "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen.tar.gz"
  )
  # Extract the correct dir
  message(STATUS "Extracting Mupen64Plus API folder")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xzvf "mupen.tar.gz" mupen64plus-core-rr-master/src/api
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp"
  )
  message(STATUS "Extracting Mupen64Plus API folder - done")
  # Clear old API files
  message(STATUS "Clearing old API headers")
  file(REMOVE_RECURSE 
    "${PROJECT_BINARY_DIR}/mupen64plus-api_download/include/mupen64plus")
  message(STATUS "Clearing old API headers - done")
  # List API headers
  message(STATUS "Filtering API headers")
  file(GLOB api_header_list 
    RELATIVE "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen64plus-core-rr-master/src/api/" 
    "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen64plus-core-rr-master/src/api/m64p_*.h"
  )
  file(MAKE_DIRECTORY 
    "${PROJECT_BINARY_DIR}/mupen64plus-api_download/include/mupen64plus")
  foreach (i IN LISTS api_header_list)
    message("moving ${i}")
    file(RENAME
      "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen64plus-core-rr-master/src/api/${i}"
      "${PROJECT_BINARY_DIR}/mupen64plus-api_download/include/mupen64plus/${i}"
    )
  endforeach()
  message(STATUS "Filtering API headers - done")
  # Clear temporary files
  file(REMOVE 
    "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen.tar.gz")
  file(REMOVE_RECURSE 
    "${PROJECT_BINARY_DIR}/CMakeFiles/CMakeTmp/mupen64plus-core-rr-master")
endif()

add_library(m64p_api_internal_do_not_use INTERFACE)
target_include_directories(m64p_api_internal_do_not_use INTERFACE "${PROJECT_BINARY_DIR}/mupen64plus-api_download/include/")

add_library(m64p::api ALIAS m64p_api_internal_do_not_use)