##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the generic_star_tracker unit tests.
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

add_cfe_coverage_stubs(generic_st-internal stubs/generic_star_tracker_device_stubs.c stubs/libuart_stubs.c) 
target_link_libraries(coverage-generic_st-internal-stubs ut_core_api_stubs ut_assert)
target_include_directories(coverage-generic_st-internal-stubs PUBLIC $<TARGET_PROPERTY:generic_st,INCLUDE_DIRECTORIES>)

 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/apps/hwlib/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/osal/ut_assert/inc)


# Add a coverage test executable called "generic_st-ALL" that 
# covers all of the functions in generic_star_tracker_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(generic_st ALL 
    "coveragetest/coveragetest_star_tracker_app.c"
    "../src/generic_star_tracker_app.c"
)

# Check if the device test file exists
add_cfe_coverage_test(generic_st DEVICE 
    "coveragetest/coveragetest_star_tracker_device.c"
    "../../shared/generic_star_tracker_device.c"
    "stubs/libuart_stubs.c"
)

add_cfe_coverage_dependency(generic_st ALL generic_st-internal)


# The generic_star_tracker uses library functions provided by generic_star_tracker_lib so must be linked
# with the generic_star_tracker_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(generic_st ALL generic_star_tracker_lib)

