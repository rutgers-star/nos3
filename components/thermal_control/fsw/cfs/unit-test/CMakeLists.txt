include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/osal/ut_assert/inc)

add_cfe_coverage_test(thermal_control CONTROL
    "coveragetest/coveragetest_thermal_control.c"
    "../src/thermal_control_app.c"
)
