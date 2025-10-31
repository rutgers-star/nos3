cmake_minimum_required(VERSION 2.6.4)
project(CFS_THERMAL_CONTROL_APP C)

include(../../../ComponentSettings.cmake)

include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)

# Need access to EPS and TMP100 message definitions
include_directories(${APPLICATION_PLATFORM_INC_LIST})

aux_source_directory(src APP_SRC_FILES)

# Create the app module
add_cfe_app(thermal_control ${APP_SRC_FILES})
