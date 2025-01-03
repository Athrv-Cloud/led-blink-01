cmake_minimum_required(VERSION 3.30)

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable code coverage in Debug mode
option(COVERAGE "Enable running with coverage" ON)

if(COVERAGE)
    message(STATUS "Enabling code coverage flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -g -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()


# Find CppUTest
set(CPPUTEST_USE_GCOV ON CACHE BOOL "Enable code coverage for CppUTest")
add_subdirectory(../tools/cpputest cpputest_build)

# Test files
set(TEST_SOURCES
    test_main.cpp        # Add more test files as needed
)

# Target for unit tests
add_executable(ledblink_tests ${TEST_SOURCES})
target_link_libraries(ledblink_tests CppUTest CppUTestExt)

# Include directories
target_include_directories(ledblink_tests PRIVATE ../src)

# Preprocessor definitions
add_definitions(-DSTM32F446xx)  # Replace with your specific device
add_definitions(-DUNIT_TEST)    # Custom definition to conditionally compile for tests

# Enable testing
enable_testing()
add_test(NAME ledblink_tests COMMAND ledblink_tests)
option(COVERAGE "Enable running with coverage" ON)



