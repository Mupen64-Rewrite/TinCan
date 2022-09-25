add_executable(rc_tool "rc.cpp")
target_compile_features(rc_tool PRIVATE cxx_std_20)
set_target_properties(rc_tool PROPERTIES
  OUTPUT_NAME rc
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tools"
)
if (NOT MSVC)
  target_compile_options(rc_tool PRIVATE "-msse4.2")
endif()

target_link_libraries(rc_tool PRIVATE fmt::fmt)