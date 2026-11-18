set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

foreach(required MK3_CC MK3_CXX MK3_SYSROOT MK3_BINUTILS_DIR)
  if("$ENV{${required}}" STREQUAL "")
    message(FATAL_ERROR "${required} is required; use scripts/mk3/build.sh")
  endif()
endforeach()

set(CMAKE_C_COMPILER "$ENV{MK3_CC}")
set(CMAKE_CXX_COMPILER "$ENV{MK3_CXX}")
set(CMAKE_SYSROOT "$ENV{MK3_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH "$ENV{MK3_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Yocto's staging compiler requires its matching assembler and linker.  The
# SDK compiler normally finds them itself; MK3_BINUTILS_DIR supports a local
# Yocto work-tree build before populate_sdk has completed.
set(CMAKE_C_FLAGS_INIT "-B$ENV{MK3_BINUTILS_DIR}/ -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mthumb -DBYTE_ORDER_LE -D_LINUX_OS_ -D_DEFAULT_SOURCE $ENV{MK3_SDK_CFLAGS}")
set(CMAKE_CXX_FLAGS_INIT "-B$ENV{MK3_BINUTILS_DIR}/ -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mthumb -DBYTE_ORDER_LE -D_LINUX_OS_ -D_DEFAULT_SOURCE $ENV{MK3_SDK_CXXFLAGS}")
if(NOT "$ENV{MK3_SOCKETCAN_ROOT}" STREQUAL "")
  set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -I$ENV{MK3_SOCKETCAN_ROOT}/include")
  set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -I$ENV{MK3_SOCKETCAN_ROOT}/include")
  set(CMAKE_MODULE_LINKER_FLAGS_INIT "-L$ENV{MK3_SOCKETCAN_ROOT}/lib")
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CFE_SYSTEM_PSPNAME "pc-linux")
set(OSAL_SYSTEM_OSTYPE "posix")
