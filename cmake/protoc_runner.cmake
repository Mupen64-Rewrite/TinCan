#!/usr/bin/cmake -P

#[=[
This CMake script executes protoc, then splits away the headers to a separate folder.
-D variables:
PROTOC: path to protoc
OUTDIR: should be ${CMAKE_CURRENT_BINARY_DIR}/${target}_protobuf
HDRDIR: should be ${CMAKE_CURRENT_BINARY_DIR}/inc
IMPDIRS: import directories for protoc
FILE: the file to compile
#]=]

execute_process(
  COMMAND "${PROTOC}" "--cpp_out=${OUTDIR}" "${FILE}" ${IMPDIRS}
  COMMAND_ERROR_IS_FATAL ANY
)

file(GLOB header_list "${OUTDIR}/*.pb.h")
foreach(header ${header_list})
  message("Found .pb.h file: ${header}")
  cmake_path(GET header FILENAME hname)
  file(COPY_FILE ${header} "${HDRDIR}/${hname}")
endforeach()