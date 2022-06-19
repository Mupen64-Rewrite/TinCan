add_library(tasinput MODULE
  "cpp-subprocess/subprocess.hpp"
  "oslib/plibdl.hpp"
  "oslib/strconv.hpp"
  "plugin.cpp"
)
target_compile_features(tasinput PRIVATE cxx_std_20)
target_link_libraries(tasinput PRIVATE m64p::api)
