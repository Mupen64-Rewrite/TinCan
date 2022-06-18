add_executable(tasinput-qt
  "oslib/plibdl.hpp"
  "oslib/strconv.hpp"
  "joystick.cpp"
  "joystick.hpp"
  "main.cpp"
  "main_window.cpp"
  "main_window.hpp"
)
target_compile_features(tasinput-qt PRIVATE cxx_std_20)
set_target_properties(tasinput-qt PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
)
target_include_directories(tasinput-qt PRIVATE "${CONFIG_DIR}")
target_link_libraries(tasinput-qt Qt6::Widgets Qt6::Gui)
target_compile_definitions(tasinput-qt PRIVATE QT_NO_KEYWORDS)