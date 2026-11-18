# Debian's armhf cross compiler, used inside the scripts/mk3 container (`make fsw-mk3`), so any
# machine with Docker can build without the GomSpace SDK. Debian 12 ships glibc 2.36, the same as
# GomSpace Linux 4.1.4 on the board, so binaries only use symbol versions the board provides.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

# Same target flags as toolchain-mk3-arm.cmake: the Zynq 7030's Cortex-A9 with NEON, hard float
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mthumb -DBYTE_ORDER_LE -D_LINUX_OS_ -D_DEFAULT_SOURCE")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -mthumb -DBYTE_ORDER_LE -D_LINUX_OS_ -D_DEFAULT_SOURCE")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CFE_SYSTEM_PSPNAME "pc-linux")
set(OSAL_SYSTEM_OSTYPE "posix")
