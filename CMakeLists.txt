cmake_policy(VERSION 3.20)
cmake_policy(SET CMP0112 NEW)

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(tincan
  LANGUAGES CXX
  DESCRIPTION "TinCan is a TASing-oriented input plugin for Mupen64Plus."
  VERSION 0.1.0
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")

if (MSVC)
  if (${VCPKG_TARGET_TRIPLET} MATCHES "-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(M64PApi)

find_package(wxWidgets REQUIRED COMPONENTS core base)
message(STATUS "wxWidgets use file: ${wxWidgets_USE_FILE}")
include(${wxWidgets_USE_FILE})

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(zeromq IMPORTED_TARGET
  "libzmq >= 4.3.4"
)


set(M64P_API_VERSION 2.1)
include(ConfigGen)


# Old UI test app
#[=========[
add_executable(tasinput-ui-test
  "src/gui/main.cpp"
  "src/gui/main_window.cpp"
  "src/gui/main_window.hpp"
  "src/gui/application.cpp"
  "src/gui/application.hpp" 
  "src/gui/buttons_panel.cpp" 
  "src/gui/buttons_panel.hpp"
  "src/gui/joystick.cpp"
  "src/gui/joystick.hpp"
  "src/gui/joystick_panel.cpp"
  "src/gui/joystick_panel.hpp"
)
target_compile_features(tasinput-ui-test PUBLIC cxx_std_20)
target_include_directories(tasinput-ui-test PRIVATE ${CONFIG_DIR})

target_link_libraries(tasinput-ui-test PUBLIC wx::wxcore m64p::api)
]=========]
add_subdirectory(tools)
add_subdirectory(src)