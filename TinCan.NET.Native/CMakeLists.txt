cmake_minimum_required(VERSION 3.22.1)
project(TinCan.NET.Native
  VERSION 0.1.0
  DESCRIPTION "Native bridge plugin for TinCan.NET"
  LANGUAGES CXX
)

if (NOT MSVC)
  string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(DEFINED VCPKG_VERBOSE)
  find_package(fmt REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem)
  find_package(cppzmq REQUIRED)
  find_package(msgpack-cxx REQUIRED)
else()
  find_package(fmt REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem)
  find_package(cppzmq CONFIG REQUIRED)
  find_package(msgpackc-cxx CONFIG REQUIRED)
endif()
include(M64PApi)

# add_executable(tester "test.cpp")
# target_link_libraries(tester PUBLIC PkgConfig::zeromq PkgConfig::msgpack)

add_subdirectory(src)