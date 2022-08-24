cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tasinput2
  LANGUAGES CXX
  VERSION 0.1.0
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(M64PApi)
include(ConfigGen)

find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)

add_executable(tasinput-ui-test
  "src/main.cpp"
  "src/main_window.cpp"
  "src/main_window.hpp"
  "src/application.cpp"
  "src/application.hpp" 
  "src/buttons_panel.cpp" 
  "src/buttons_panel.hpp"
  "src/joystick.cpp"
  "src/joystick.hpp"
)
target_compile_features(tasinput-ui-test PUBLIC cxx_std_20)

target_link_libraries(tasinput-ui-test PUBLIC wx::wxcore m64p::api)