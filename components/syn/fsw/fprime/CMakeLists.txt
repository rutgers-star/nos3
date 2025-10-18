cmake_minimum_required(VERSION 2.6.4)
project(CFS_SYN_APP C)

include(../../ComponentSettings.cmake)
add_subdirectory(../synopsis ..)

set(CMAKE_C_FLAGS "-Wno-format")
set(CMAKE_CXX_FLAGS "-Wno-format")

include_directories(public_inc)
include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)
include_directories(../synopsis/include)

#include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)

file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${TGTSYS_${SYSVAR}}/data)
file(COPY owls DESTINATION ${CMAKE_INSTALL_PREFIX}/${TGTSYS_${SYSVAR}}/data/)
# Set RPATH that is getting stripped by CMAKE
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${TGTSYS_${SYSVAR}}/${INSTALL_SUBDIR}")

# Create the app module
aux_source_directory(src APP_SRC_FILES)
add_cfe_app(syn ${APP_SRC_FILES})

target_link_libraries(syn synopsis)
