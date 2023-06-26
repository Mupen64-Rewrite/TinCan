function(parse_script_args)
  math(EXPR last_arg "${CMAKE_ARGC} - 1")
  foreach(i RANGE ${last_arg})
    if ("${CMAKE_ARGV${i}}" MATCHES "^-P")
      string(LENGTH "${CMAKE_ARGV${i}}" arg_len)
      # if 
    endif()
  endforeach()
endfunction()

parse_script_args()