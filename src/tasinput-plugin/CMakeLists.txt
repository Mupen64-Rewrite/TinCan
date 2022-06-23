add_library(tasinput MODULE
  "oslib/plibdl.hpp"
  "oslib/strconv.hpp"
  "subp/subprocess.hpp"
  "plugin.cpp"
)
target_include_directories(tasinput PRIVATE "${CONFIG_DIR}" "${PROJECT_BINARY_DIR}/mupen64plus-api_download/include")

target_compile_features(tasinput PRIVATE cxx_std_20)
# target_link_libraries(tasinput PRIVATE m64p::api)
