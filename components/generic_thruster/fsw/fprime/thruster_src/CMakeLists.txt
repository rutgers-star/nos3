####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
# UT_SOURCE_FILES: list of source files for unit tests
#
####
include_directories("../../shared")
include_directories("../../standalone") #device_cfg.h
include_directories("../../../../../fsw/apps/hwlib/fsw/public_inc")
include_directories("../platform_inc")
include_directories("../../../../../fsw/apps/hwlib/sim/inc")






set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/Generic_thruster.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/Generic_thruster.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../shared/generic_thruster_device.c"
  "${CMAKE_CURRENT_LIST_DIR}/../../../../../fsw/apps/hwlib/sim/src/nos_link.c"
)

# Uncomment and add any modules that this component depends on, else
# they might not be available when cmake tries to build this component.

set(MOD_DEPS
  ${ITC_Common_LIBRARIES}
  ${NOSENGINE_LIBRARIES}
)

register_fprime_module()
