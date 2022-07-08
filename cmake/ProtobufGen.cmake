function(list_sources target)
  get_target_property(_srcs ${target} SOURCES)
  message(STATUS "sources for ${target}: ${_srcs}")
endfunction()

#[========[
gen_protobuf(
  TARGET <target> [OUTPUT_DIR <dir>] [HEADER_DIR <dir>]
  [PLUGINS <name> <target or path> ...])
#]========]
function(gen_protobuf)
  set(_arg_opts)
  set(_arg_single TARGET OUTPUT_DIR HEADER_DIR)
  set(_arg_multi PLUGINS)
  
  cmake_parse_arguments(PARSE_ARGV 0 gen_pb 
    "${_arg_opts}" "${_arg_single}" "${_arg_multi}")
  
  if (NOT DEFINED gen_pb_TARGET)
    message(SEND_ERROR "gen_protobuf() received no target.")
  endif()
  
  # Setup output directory
  if (DEFINED gen_pb_OUTPUT_DIR)
    set(_outdir "${gen_pb_OUTPUT_DIR}")
  endif()
  if(NOT DEFINED _outdir)
    set(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${gen_pb_TARGET}_protobuf")
  endif()
  
  file(MAKE_DIRECTORY "${_outdir}")
  
  set(_hdrdir "${_outdir}/inc")

  if(DEFINED gen_pb_HEADER_DIR)
    set(_hdrdir "${_hdrdir}/${gen_pb_HEADER_DIR}")
  endif()

  file(MAKE_DIRECTORY "${_hdrdir}")

  # List .proto files
  get_target_property(_target_srcs ${gen_pb_TARGET} SOURCES)
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
  
  # Setup plugins
  if (DEFINED gen_pb_PLUGINS)
    list(LENGTH gen_pb_PLUGINS _plugin_list_len)
    math(EXPR _plugin_list_check "${_plugin_list_len} % 2")
    if(NOT _plugin_list_check EQUAL 0)
      message(SEND_ERROR "gen_protobuf() received uneven number of plugin args.")
    endif()
    
    set(_plugin_args)
    set(_plugin_deps)
    
    math(EXPR _plugin_list_len "${_plugin_list_len} - 1")
    foreach(i RANGE 0 ${_plugin_list_len} 2)
      list(GET gen_pb_PLUGINS ${i} _plugin_name)
      math(EXPR j "${i} + 1")
      list(GET gen_pb_PLUGINS ${j} _plugin_path)
      
      list(APPEND _plugin_deps ${_plugin_path})
      
      if (NOT EXISTS ${_plugin_path} AND TARGET ${_plugin_path})
        set(_plugin_path "$<TARGET_FILE_DIR:${_plugin_path}>/$<TARGET_FILE_NAME:${_plugin_path}>")
      endif()
      
      list(APPEND _plugin_args "--plugin=protoc-gen-${_plugin_name}=${_plugin_path}" "--${_plugin_name}_out=${_outdir}")
    endforeach()
  endif()

  set(_protoc_exts ".pb.cc" ".pb.h")
  set(_protoc_gensrcs_all)

  foreach(_file ${_protoc_srcs})
    cmake_path(GET _file STEM LAST_ONLY _file_stem)

    # List out generated files
    set(_file_outputs)

    foreach(_ext ${_protoc_exts})
      list(APPEND _file_outputs "${_outdir}/${_file_stem}${_ext}")
      list(APPEND _protoc_gensrcs_all "${_outdir}/${_file_stem}${_ext}")
    endforeach()

    get_target_property(_protoc_exe protobuf::protoc IMPORTED_LOCATION)

    # protoc command
    add_custom_command(
      OUTPUT ${_file_outputs}
      COMMAND "${CMAKE_COMMAND}"
      ARGS "-DOUTDIR=${_outdir}" "-DHDRDIR=${_hdrdir}" -P "${tas-input-qt_SOURCE_DIR}/cmake/protoc_runner.cmake" 
        "${_protoc_exe}" "${_file}" "--cpp_out=${_outdir}" ${_plugin_args} ${_import_dirs}
      DEPENDS ${_file} protobuf::protoc "${tas-input-qt_SOURCE_DIR}/cmake/protoc_runner.cmake" ${_plugin_deps}
      COMMENT "Compiling C++ protocol buffer for ${_file}"
    )
  endforeach()

  # mark all as generated
  set_source_files_properties(${_protoc_gensrcs_all} PROPERTIES GENERATED TRUE)

  # append srcs
  target_sources(${gen_pb_TARGET} PRIVATE ${_protoc_gensrcs_all})
  target_include_directories(${gen_pb_TARGET} PUBLIC ${_hdrdir})
endfunction()
