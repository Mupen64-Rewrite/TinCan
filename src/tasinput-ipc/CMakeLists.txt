add_library(tasinput-ipc STATIC
  "src/shmipc.proto"
  "src/shm_server.cpp"
)
gen_protobuf(tasinput-ipc)
list_sources(tasinput-ipc)

target_include_directories(tasinput-ipc PUBLIC inc "${CMAKE_CURRENT_BINARY_DIR}/inc")
target_compile_features(tasinput-ipc PUBLIC cxx_std_20)
target_link_libraries(tasinput-ipc PUBLIC Boost::boost protobuf::libprotobuf)