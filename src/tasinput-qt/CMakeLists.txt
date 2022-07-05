add_executable(tasinput-qt
  "src/joystick.cpp"
  "src/joystick.hpp"
  "src/main.cpp"
  "src/main_window.cpp"
  "src/main_window.hpp"
  "src/aspect_layout.hpp"
)
target_compile_features(tasinput-qt PRIVATE cxx_std_20)
set_target_properties(tasinput-qt PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
)
target_include_directories(tasinput-qt PRIVATE "${CONFIG_DIR}")
target_link_libraries(tasinput-qt PRIVATE Qt6::Widgets Qt6::Gui m64p::api tasinput-ipc)
target_compile_definitions(tasinput-qt PRIVATE QT_NO_KEYWORDS)

# windeployqt is needed to get all dlls for deployment
if (MSVC)
  add_custom_command(TARGET tasinput-qt POST_BUILD
    COMMAND cmd /c "$<IF:$<CONFIG:Debug>,${TOOL_WINDEPLOYQT_Debug},${TOOL_WINDEPLOYQT_Release}>"
    $<TARGET_FILE:tasinput-qt>
    COMMENT "Running ${TOOL_WINDEPLOYQT_Debug}"
  )
endif()