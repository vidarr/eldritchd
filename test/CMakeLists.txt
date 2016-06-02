include_directories("${eldritch_BINARY_DIR}" ${eldritch_SOURCE_DIR}/src)
add_executable(test_url test_url.c)
target_link_libraries(test_url eldritch)
