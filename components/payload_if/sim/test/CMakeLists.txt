# Standalone unit tests for the PayOBC link model. They need neither NOS Engine
# nor ground software, only a C/C++ compiler and GoogleTest (both are in the
# NOS3 build image). See ../README.md for the build and run commands.
cmake_minimum_required(VERSION 3.13)
project(payload_if_sim_test C CXX)

find_package(GTest REQUIRED)
enable_testing()

# The same pinned submodules the BusOBC FSW builds against
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../fsw/payload-link ${CMAKE_CURRENT_BINARY_DIR}/payload-link)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../fsw/payload-apids ${CMAKE_CURRENT_BINARY_DIR}/payload-apids)

# Match the warning flags the NOS3 simulator build uses (sims/CMakeLists.txt)
add_executable(test_payobc_link_model
    test_payobc_link_model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/payobc_link_model.cpp
)
set_target_properties(test_payobc_link_model PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
target_include_directories(test_payobc_link_model PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
target_compile_options(test_payobc_link_model PRIVATE
    -Werror -Wall -Wextra -pedantic -Wno-vla -Wwrite-strings -Wpointer-arith -Wcast-align)
target_link_libraries(test_payobc_link_model PRIVATE payload_link STAR::payload_apids GTest::gtest GTest::gtest_main)

add_test(NAME payobc_link_model COMMAND test_payobc_link_model)
