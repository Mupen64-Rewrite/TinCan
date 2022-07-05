function(list_sources target)
  get_target_property(_srcs ${target} SOURCES)
  message(STATUS "sources for ${target}: ${_srcs}")
endfunction()

function(gen_protobuf target)
  # Setup output directory
  set(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${target}_protobuf")
  file(MAKE_DIRECTORY "${_outdir}")
  set(_hdrdir "${_outdir}/inc")

  if(${ARGC} GREATER_EQUAL 2)
    set(_hdrdir "${_hdrdir}/${ARG2}")
  endif()

  file(MAKE_DIRECTORY "${_hdrdir}")

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
      ARGS "-DOUTDIR=${_outdir}" "-DHDRDIR=${_hdrdir}" -P "${tas-input-qt_SOURCE_DIR}/cmake/protoc_runner.cmake" "${_protoc_exe}" "${_file}" "--cpp_out=${_outdir}" ${_import_dirs}
      DEPENDS ${_file} protobuf::protoc "${tas-input-qt_SOURCE_DIR}/cmake/protoc_runner.cmake"
      COMMENT "Compiling C++ protocol buffer for ${_file}"
    )
  endforeach()

  # mark all as generated
  set_source_files_properties(${_protoc_gensrcs_all} PROPERTIES GENERATED TRUE)

  # append srcs
  target_sources(${target} PRIVATE ${_protoc_gensrcs_all})
  target_include_directories(${target} PUBLIC ${_hdrdir})
endfunction()
