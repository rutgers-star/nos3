##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the generic_imu unit tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

#
#
# NOTE on the subdirectory structures here:
#
# - "inc" provides local header files shared between the coveragetest,
#    wrappers, and overrides source code units
# - "coveragetest" contains source code for the actual unit test cases
#    The primary objective is to get line/path coverage on the FSW 
#    code units.
#
 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/fsw/shared)


# Add a coverage test executable called "generic_imu-ALL" that 
# covers all of the functions in generic_imu_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(generic_imu ALL 
    "coveragetest/coveragetest_generic_imu_app.c"
    "../src/generic_imu_app.c"
    "../shared/generic_imu_device.c"
    "../../../../fsw/apps/hwlib/fsw/stubs/libuart.c"
    "../../../../fsw/apps/hwlib/fsw/stubs/libcan.c"
)

# The generic_imu uses library functions provided by generic_imu_lib so must be linked
# with the generic_imu_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(generic_imu ALL generic_imu_lib)

