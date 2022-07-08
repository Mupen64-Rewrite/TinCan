#!/usr/bin/cmake -P

#[=[
This CMake script executes protoc, then copies the headers to a separate folder.
-D variables:
PROTOC: path to protoc
OUTDIR: should be ${CMAKE_CURRENT_BINARY_DIR}/${target}_protobuf
HDRDIR: should be ${CMAKE_CURRENT_BINARY_DIR}/inc
FILE: the file to compile
#]=]

math(EXPR argv_end "${CMAKE_ARGC} - 1")
set(tmp0 2)
set(args)
# bad argument collector
foreach(i RANGE 1 ${argv_end})
  if(tmp0 EQUAL 0)
    list(APPEND args ${CMAKE_ARGV${i}})
  elseif(tmp0 EQUAL 2)
    if (${CMAKE_ARGV${i}} STREQUAL "-P")
      set(tmp0 1)
    endif()
  elseif(tmp0 EQUAL 1)
    set(tmp0 0)
  endif()
endforeach()


execute_process(
  COMMAND ${args}
  COMMAND_ERROR_IS_FATAL ANY
)

file(GLOB header_list "${OUTDIR}/*.pb.h")
foreach(header ${header_list})
  message("Moving ${header}")
  cmake_path(GET header FILENAME hname)
  file(COPY_FILE ${header} "${HDRDIR}/${hname}")
endforeach()