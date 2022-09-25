set(OSLIB_SOURCES
  "oslib/details/convert.hpp"
  "oslib/details/meta.hpp"
  "oslib/dynlib.hpp"
  "oslib/preproc.hpp"
  "oslib/process.hpp"
  "oslib/shmem.hpp"
)

add_executable(tasinput2-ui WIN32
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

target_compile_features(tasinput2-ui PUBLIC cxx_std_20)
target_include_directories(tasinput2-ui PRIVATE ${CONFIG_DIR})
target_link_libraries(tasinput2-ui PUBLIC ${wxWidgets_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads m64p::api)

# rc-based generator for resdata_exedata.cpp
if (NOT IS_DIRECTORY "${PROJECT_BINARY_DIR}/tools/rc-gen")
  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tools/rc-gen")
endif()
add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.cpp" "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.hpp"
  COMMAND rc_tool $<TARGET_FILE:tasinput2-ui> "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.cpp" 
    tasinput exedata "${PROJECT_BINARY_DIR}/tools/rc-gen/resdata_exedata.hpp"
)

add_library(tasinput2 MODULE
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
target_include_directories(tasinput2 PRIVATE "${PROJECT_BINARY_DIR}/tools/rc-gen")
target_compile_features(tasinput2 PUBLIC cxx_std_20)
target_include_directories(tasinput2 PRIVATE ${CONFIG_DIR})

target_link_libraries(tasinput2 PUBLIC ${wxWidgets_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads m64p::api)
