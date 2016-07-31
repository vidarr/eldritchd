cmake_minimum_required (VERSION 3.0)
project (eldritch_src
         VERSION 1.0)


# add the binary tree to the search path for include files
include_directories("${eldritch_BINARY_DIR}")

set (
    eldritch_src_source_files
    forker.c
    hash.c
    http.c
    utils.c
    url.c
    contenttype.c
    configuration.c
)

set (
    eldritch_src_header_files
    config.h
    forker.h
    hash.h
    http.h
    utils.h
    url.h
    contenttype.h
    configuration.h
)
add_library(eldritch ${eldritch_src_source_files} ${eldritch_src_header_files})
# add the executable
add_executable(eldritchd eldritch.c)
target_link_libraries(eldritchd eldritch)
