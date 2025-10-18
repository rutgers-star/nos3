cmake_minimum_required(VERSION 2.6.4)
project(CFS_NOVATEL_OEM615 C)

include(../../ComponentSettings.cmake)

include_directories(mission_inc)
include_directories(platform_inc)
include_directories(../shared)

aux_source_directory(src APP_SRC_FILES)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)

# Create the app module
add_cfe_app(novatel_oem615 ${APP_SRC_FILES}
            ../shared/novatel_oem615_device.c)
