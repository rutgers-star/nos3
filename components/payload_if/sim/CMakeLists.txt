project(payload_if_sim)

find_package(ITC_Common REQUIRED QUIET COMPONENTS itc_logger)
find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client uart)

include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ${ITC_Common_INCLUDE_DIRS}
                    ${NOSENGINE_INCLUDE_DIRS})

set(payload_if_sim_src
    src/payload_if_hardware_model.cpp
    src/payload_if_42_data_provider.cpp
    src/payload_if_data_provider.cpp
    src/payload_if_data_point.cpp
)

# For Code::Blocks and other IDEs
file(GLOB payload_if_sim_inc inc/*.hpp)

set(payload_if_sim_libs
    sim_common
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib") # Pick up .so in install directory

add_library(payload_if_sim SHARED ${payload_if_sim_src} ${payload_if_sim_inc})
target_link_libraries(payload_if_sim ${payload_if_sim_libs})
install(TARGETS payload_if_sim LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
