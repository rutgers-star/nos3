##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the mgr unit tests.
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

add_cfe_coverage_stubs(mgr-internal stubs/mgr_device_stubs.c) 
target_link_libraries(coverage-mgr-internal-stubs ut_core_api_stubs ut_assert)
target_include_directories(coverage-mgr-internal-stubs PUBLIC $<TARGET_PROPERTY:mgr,INCLUDE_DIRECTORIES>)

 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)


# Add a coverage test executable called "mgr-ALL" that 
# covers all of the functions in mgr_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(mgr ALL 
    "coveragetest/coveragetest_mgr_app.c"
    "../src/mgr_app.c"
)

add_cfe_coverage_dependency(mgr ALL mgr-internal)


# The mgr uses library functions provided by mgr_lib so must be linked
# with the mgr_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(mgr ALL mgr_lib)

