##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the tmp100 unit tests.
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

add_cfe_coverage_stubs(tmp100-internal stubs/tmp100_device_stubs.c stubs/libuart_stubs.c) 
target_link_libraries(coverage-tmp100-internal-stubs ut_core_api_stubs ut_assert)
target_include_directories(coverage-tmp100-internal-stubs PUBLIC $<TARGET_PROPERTY:tmp100,INCLUDE_DIRECTORIES>)

 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/osal/ut_assert/inc)


# Add a coverage test executable called "tmp100-ALL" that 
# covers all of the functions in tmp100_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(tmp100 ALL 
    "coveragetest/coveragetest_tmp100_app.c"
    "../src/tmp100_app.c"
)

add_cfe_coverage_test(tmp100 DEVICE 
    "coveragetest/coveragetest_tmp100_device.c"
    "../../shared/tmp100_device.c"
    "stubs/libuart_stubs.c"
)

add_cfe_coverage_dependency(tmp100 ALL tmp100-internal)


# The tmp100 uses library functions provided by tmp100_lib so must be linked
# with the tmp100_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(tmp100 ALL tmp100_lib)

