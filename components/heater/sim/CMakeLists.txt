project(heater_sim)

find_package(Boost REQUIRED QUIET COMPONENTS system filesystem)
find_package(ITC_Common REQUIRED QUIET COMPONENTS itc_logger)
find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client i2c)

include_directories(inc
                    ${sim_common_SOURCE_DIR}/inc
                    ${ITC_Common_INCLUDE_DIRS}
                    ${NOSENGINE_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS})

set(heater_sim_src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/heater_hardware_model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/heater_data_point.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/heater_data_provider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/heater_42_data_provider.cpp
)

# For Code::Blocks and other IDEs
file(GLOB heater_sim_inc ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp)

set(heater_sim_libs
    ${Boost_LIBRARIES}
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib")
add_library(heater_sim SHARED ${heater_sim_src} ${heater_sim_inc})
target_link_libraries(heater_sim sim_common ${heater_sim_libs})
install(TARGETS heater_sim LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
