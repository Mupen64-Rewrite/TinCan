set(OSLIB_SOURCES
  "oslib/details/convert.hpp"
  "oslib/details/meta.hpp"
  "oslib/dynlib.hpp"
  "oslib/preproc.hpp"
  "oslib/process.hpp"
  "oslib/shmem.hpp"
)

add_executable(tincan-ui WIN32
  ${OSLIB_SOURCES}
  "gui/main_window.cpp"
  "gui/main_window.hpp"
  "gui/application.cpp"
  "gui/application.hpp" 
  "gui/buttons_panel.cpp" 
  "gui/buttons_panel.hpp"
  "gui/joystick.cpp"
  "gui/joystick.hpp"
  "gui/joystick_panel.cpp"
  "gui/joystick_panel.hpp"
)

target_compile_features(tincan-ui PUBLIC cxx_std_20)
target_include_directories(tincan-ui PRIVATE ${CONFIG_DIR})
target_link_libraries(tincan-ui PUBLIC ${wxWidgets_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads m64p::api)

# rc-based generator for resdata_exedata.cpp
if (NOT IS_DIRECTORY "${PROJECT_BINARY_DIR}/tools/rc-gen")
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tools/rc-gen")
endif()
add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.cpp" "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.hpp"
  COMMAND rc_tool $<TARGET_FILE:tincan-ui> "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.cpp" 
    tasinput exedata "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.hpp"
)

add_library(tincan MODULE
  ${OSLIB_SOURCES}
  "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.cpp"
  "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.hpp"
  "resdata.hpp"
  "core_fns.cpp"
  "core_fns.hpp"
  "global.cpp"
  "global.hpp"
  "plugin.cpp"
)
target_include_directories(tincan PRIVATE "${PROJECT_BINARY_DIR}/tools/rc-gen")
target_compile_features(tincan PUBLIC cxx_std_20)
target_include_directories(tincan PRIVATE ${CONFIG_DIR})

target_link_libraries(tincan PUBLIC ${wxWidgets_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads m64p::api)
