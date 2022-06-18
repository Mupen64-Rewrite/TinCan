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

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)

add_subdirectory(src/tasinput-qt)
add_subdirectory(src/tasinput-plugin)
