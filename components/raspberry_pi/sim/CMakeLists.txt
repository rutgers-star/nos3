project(raspberry_pi_sim)

find_package(ITC_Common REQUIRED QUIET COMPONENTS itc_logger)
find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client uart)

include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ${ITC_Common_INCLUDE_DIRS}
                    ${NOSENGINE_INCLUDE_DIRS})

set(raspberry_pi_sim_src
    src/raspberry_pi_hardware_model.cpp
    src/raspberry_pi_42_data_provider.cpp
    src/raspberry_pi_data_provider.cpp
    src/raspberry_pi_data_point.cpp
)

# For Code::Blocks and other IDEs
file(GLOB raspberry_pi_sim_inc inc/*.hpp)

set(raspberry_pi_sim_libs
    sim_common
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib") # Pick up .so in install directory

add_library(raspberry_pi_sim SHARED ${raspberry_pi_sim_src} ${raspberry_pi_sim_inc})
target_link_libraries(raspberry_pi_sim ${raspberry_pi_sim_libs})
install(TARGETS raspberry_pi_sim LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
