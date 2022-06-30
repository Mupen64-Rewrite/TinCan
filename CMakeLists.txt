cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tas-input-qt
  LANGUAGES CXX
  VERSION 0.1.0
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(M64PApi)
include(ConfigGen)
include(ProtobufGen)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

# Need windeployqt on Windows, not sure about MacOS
if (MSVC)
  find_program(TOOL_WINDEPLOYQT_Debug windeployqt.debug.bat)
  find_program(TOOL_WINDEPLOYQT_Release windeployqt)

  message(STATUS "Found windeployqt (dbg): ${TOOL_WINDEPLOYQT_Debug}")
  message(STATUS "Found windeployqt (rel): ${TOOL_WINDEPLOYQT_Release}")
endif()
add_subdirectory(src/tasinput-ipc)
add_subdirectory(src/tasinput-qt)
add_subdirectory(src/tasinput-plugin)