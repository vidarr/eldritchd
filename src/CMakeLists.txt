cmake_minimum_required (VERSION 3.0)
project (eldritch_src
         VERSION 1.0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "config.h.in"
    "${eldritch_BINARY_DIR}/config.h"
  )

# add the binary tree to the search path for include files
include_directories("${eldritch_BINARY_DIR}")

set (
    eldritch_src_source_files
    forker.c
    http.c
    utils.c
)
# add the executable
add_executable(eldritch eldritch.c ${eldritch_src_source_files})
