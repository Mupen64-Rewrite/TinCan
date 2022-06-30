function(list_sources target)
  get_target_property(_srcs ${target} SOURCES)
  message(STATUS "sources for ${target}: ${_srcs}")
endfunction()

function(gen_protobuf target)
  # Setup output directory
  set(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${target}_protobuf")
  file(MAKE_DIRECTORY "${_outdir}")
  
  # List .proto files
  get_target_property(_target_srcs ${target} SOURCES)
  set(_protoc_srcs)
  foreach(_file ${_target_srcs})
    if(_file MATCHES "proto$")
      message("Found protobuf file: ${_file}")
      cmake_path(ABSOLUTE_PATH _file OUTPUT_VARIABLE _abs)
      list(APPEND _protoc_srcs "${_abs}")
    endif()
  endforeach()
  
  # Setup import directories, because protoc is 
  # too dumb to read absolute paths
  set(_import_dirs)
  foreach(_file ${_protoc_srcs})
    cmake_path(GET _file PARENT_PATH _dir)
    list(APPEND _import_dirs "-I${_dir}")
  endforeach()
  
  set(_protoc_exts ".pb.cc" ".pb.h")
  set(_protoc_gensrcs_all)
  foreach (_file ${_protoc_srcs})
    cmake_path(GET _file STEM LAST_ONLY _file_stem)
    
    # List out generated files
    set(_file_outputs)
    foreach (_ext ${_protoc_exts})
      list(APPEND _file_outputs "${_outdir}/${_file_stem}${_ext}")
      list(APPEND _protoc_gensrcs_all "${_outdir}/${_file_stem}${_ext}")
    endforeach()
    
    # protoc command
    add_custom_command(
      OUTPUT ${_file_outputs}
      COMMAND protobuf::protoc
      ARGS "--cpp_out=${_outdir}" "${_file}" ${_import_dirs}
      DEPENDS ${_file} protobuf::protoc
      COMMENT "Compiling C++ protocol buffer for ${_file}"
    )
  endforeach()
  
  # mark all as generated
  set_source_files_properties(${_protoc_gensrcs_all} PROPERTIES GENERATED TRUE)
  # append srcs
  target_sources(${target} PRIVATE ${_protoc_gensrcs_all})
  target_include_directories(${target} PRIVATE ${_outdir})
endfunction()
