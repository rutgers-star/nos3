cmake_minimum_required(VERSION 2.6.4)
project(CFS_GENERIC_THRUSTER_APP C)

include(../../../ComponentSettings.cmake)

include_directories(public_inc)
include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)

include_directories(../shared)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)

aux_source_directory(src APP_SRC_FILES)

# Create the app module
add_cfe_app(generic_thruster ${APP_SRC_FILES}
            ../shared/generic_thruster_device.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
