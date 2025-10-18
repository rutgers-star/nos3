cmake_minimum_required(VERSION 2.6.4)
project(CFS_IO_LIB C)

include_directories(fsw/public_inc)

aux_source_directory(fsw/src APP_SRC_FILES)
aux_source_directory(fsw/src/formats APP_SRC_FILES)
aux_source_directory(fsw/src/services APP_SRC_FILES)

include_directories(${MISSION_SOURCE_DIR}/../components/cryptolib/include)

# Create the app module
add_cfe_app(io_lib ${APP_SRC_FILES})
