##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the generic_css unit tests.
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
 

add_cfe_coverage_stubs(generic_css-internal stubs/generic_css_device_stubs.c stubs/libi2c_stubs.c) 
target_link_libraries(coverage-generic_css-internal-stubs ut_core_api_stubs ut_assert)
target_include_directories(coverage-generic_css-internal-stubs PUBLIC $<TARGET_PROPERTY:generic_css,INCLUDE_DIRECTORIES>)



# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/osal/ut_assert/inc)



# Add a coverage test executable called "generic_css-ALL" that 
# covers all of the functions in generic_css_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(generic_css ALL 
    "coveragetest/coveragetest_generic_css_app.c"
    "../src/generic_css_app.c"
    #"../../shared/generic_css_device.c"
    #"../../../../../fsw/apps/hwlib/fsw/stubs/libi2c.c"
)
add_cfe_coverage_test(generic_css DEVICE 
    "coveragetest/coveragetest_generic_css_device.c"
    "../../shared/generic_css_device.c"
    "stubs/libi2c_stubs.c"
)
# The generic_css uses library functions provided by generic_css_lib so must be linked
# with the generic_css_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(generic_css ALL generic_css_lib)
add_cfe_coverage_dependency(generic_css ALL generic_css-internal)
