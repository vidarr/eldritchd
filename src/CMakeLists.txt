cmake_minimum_required (VERSION 3.0)
project (eldritch_src
         VERSION 1.0)


# add the binary tree to the search path for include files
include_directories("${eldritch_BINARY_DIR}")

set (
    eldritch_src_source_files
    forker.c
    http.c
    utils.c
)

set (
    eldritch_src_header_files
    config.h
    forker.h
    http.h
    utils.h
)
# add the executable
add_executable(eldritch eldritch.c ${eldritch_src_source_files}
                                   ${eldritch_src_header_files})
