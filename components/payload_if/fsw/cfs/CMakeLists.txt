cmake_minimum_required(VERSION 2.6.4)
project(CFS_PAYLOAD_IF_APP C)

include(../../../ComponentSettings.cmake)

include_directories(public_inc)
include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)

include_directories(../shared)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${APPLICATION_PLATFORM_INC_LIST})
include_directories(../payload-link/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../payload-link ${CMAKE_CURRENT_BINARY_DIR}/payload-link)
include_directories(../payload-apids/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../payload-apids ${CMAKE_CURRENT_BINARY_DIR}/payload-apids)

aux_source_directory(src APP_SRC_FILES)

# Create the app module
add_cfe_app(payload_if ${APP_SRC_FILES}
            ../shared/payload_if_device.c)
target_link_libraries(payload_if payload_link)
target_link_libraries(payload_if STAR::payload_apids)
# payload-link is a pinned external dependency (commit 0a8e90d); demote this
# warning to avoid patching the vendored source, matching the pattern used
# for io_lib/hwlib in arch_build_custom.cmake.
target_compile_options(payload_link PRIVATE -Wno-error=switch-default)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
