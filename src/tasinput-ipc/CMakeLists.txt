add_library(tasinput-ipc STATIC
  "src/tnp_ipc.proto"
  "src/tnp_prtc.proto"
  "src/dummy.cpp"
)
gen_protobuf(tasinput-ipc)

set_target_properties(tasinput-ipc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)
target_include_directories(tasinput-ipc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc" "${CMAKE_CURRENT_BINARY_DIR}/inc")
target_compile_features(tasinput-ipc PUBLIC cxx_std_20)
target_link_libraries(tasinput-ipc PUBLIC Boost::boost protobuf::libprotobuf)