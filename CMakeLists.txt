cmake_minimum_required (VERSION 3.0)
project (eldritch
         VERSION 1.0.0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "src/config.h.in"
    "${eldritch_BINARY_DIR}/config.h"
)

add_custom_target(
    buildnum
    COMMAND bash retag_build.sh "${eldritch_BINARY_DIR}/config.h"
    WORKING_DIRECTORY "${eldritch_SOURCE_DIR}/bin"
)

enable_testing()
add_subdirectory("test")
add_subdirectory("src")

add_test(NAME testUrl COMMAND test_url)
