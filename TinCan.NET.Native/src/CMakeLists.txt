macro(tc_link_dependencies target)
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_link_libraries(${target} PUBLIC ${CMAKE_DL_LIBS})
  if(DEFINED VCPKG_VERBOSE)
    target_link_libraries(${target} PUBLIC Boost::boost Boost::filesystem fmt::fmt cppzmq msgpack-cxx m64p::api)
  else()
    target_link_libraries(${target} PUBLIC Boost::boost Boost::filesystem fmt::fmt cppzmq msgpackc-cxx m64p::api)
  endif()
  if(MSVC)
    set_target_properties(${target} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    target_link_libraries(${target} PRIVATE bcrypt.lib)
  endif()
  if(WIN32)
    target_compile_definitions(${target} PUBLIC WIN32_LEAN_AND_MEAN)
  endif()
endmacro()

add_library(tincan-bridge MODULE
  "ipc/postbox.hpp"
  "ipc/postbox.cpp"
  "ipc/safe_queue.hpp"
  
  "tl/function_ref.hpp"
  
  "util/fs_helper.cpp"
  "util/fs_helper.hpp"
  "util/bin_cast.hpp"
  "util/gate.hpp"
  "util/string_hash.hpp"
  "util/secrand.cpp"
  "util/secrand.hpp"
  
  "plugin.cpp"
  "global.cpp"
  "global.hpp"
)
tc_link_dependencies(tincan-bridge)
