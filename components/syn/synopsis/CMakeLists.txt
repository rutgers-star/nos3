cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
project(synopsis DESCRIPTION "Science Yield improvemeNt via Onboard Prioritization and Summary of Information System")

option(SYNOPSIS_TEST "Synopsis Test" OFF)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "-Wnon-virtual-dtor")
set(CMAKE_C_FLAGS "-Wno-format")
set(CMAKE_CXX_FLAGS "-Wno-format")

#FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
#FetchContent_MakeAvailable(json)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

if(SYNOPSIS_TEST)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_library(synopsis SHARED
src/sqlite3.c
src/Sqlite3Statement.cpp
src/synopsis.cpp
src/ASDS.cpp
src/DpMsg.cpp
src/DpDbMsg.cpp
src/PassthroughASDS.cpp
src/SqliteASDPDB.cpp
src/StdLogger.cpp
src/LinuxClock.cpp
src/Timer.cpp
src/RuleAST.cpp
src/DownlinkPlanner.cpp
src/MaxMarginalRelevanceDownlinkPlanner.cpp
src/Similarity.cpp
src/itc_synopsis_bridge.cpp
)
set_target_properties(synopsis PROPERTIES PUBLIC_HEADER include/synopsis.hpp)
target_include_directories(synopsis PRIVATE include)
target_include_directories(synopsis PRIVATE src)
target_link_libraries(synopsis PRIVATE nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})

include(GNUInstallDirs)
install(TARGETS synopsis
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# CLI
add_executable(
synopsis_cli
bin/synopsis_cli.cpp
)
target_include_directories(synopsis_cli PRIVATE include)
target_link_libraries(
synopsis_cli PRIVATE nlohmann_json::nlohmann_json
synopsis
pthread
)

# Unit Testing
if(SYNOPSIS_TEST)
    enable_testing()

    add_executable(
    synopsis_test
    test/synopsis_test.cpp
    )
    target_include_directories(synopsis_test PRIVATE include)
    target_link_libraries(
    synopsis_test
    gtest_main
    synopsis
    )

    include(GoogleTest)
    gtest_discover_tests(synopsis_test PROPERTIES ENVIRONMENT
    "SYNOPSIS_TEST_DATA=${CMAKE_CURRENT_LIST_DIR}/test/data"
    )
endif()

install(TARGETS synopsis DESTINATION ${CMAKE_INSTALL_PREFIX}/${TGTSYS_${SYSVAR}}/${INSTALL_SUBDIR})
