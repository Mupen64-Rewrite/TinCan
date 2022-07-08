add_library(tasinput MODULE
  "src/oslib/plibdl.hpp"
  "src/oslib/strconv.hpp"
  "src/plugin.cpp"
  "src/stubs.cpp"
)
target_include_directories(tasinput PRIVATE "${CONFIG_DIR}")

target_compile_features(tasinput PRIVATE cxx_std_20)
target_link_libraries(tasinput PRIVATE m64p::api tasinput-ipc Boost::filesystem)