cmake_minimum_required(VERSION 3.15)
project(BlackBoxTests)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES "Src/*.cpp" "Src/*.h")

add_executable(unit_tests
    ${TEST_SOURCES}
    ../Core/App/Sensor/Barometer/Barometer.cpp
    ../Core/App/Sensor/GPS/Gps.cpp
    ../Core/App/Logger/Impl/FatFSLogger.cpp
    ../Core/App/Service/MavLinkSender.cpp
)

target_compile_definitions(unit_tests PRIVATE UNIT_TESTING)

target_include_directories(unit_tests PRIVATE
    ../Core/App
    Src
    ../Middlewares/Third_Party/mavlink
)

target_link_libraries(unit_tests
    gtest_main
    gtest
    gmock
    gmock_main
)

enable_testing()
add_test(NAME AllTests COMMAND unit_tests)