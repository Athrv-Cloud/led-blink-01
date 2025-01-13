cmake_minimum_required(VERSION 3.30)
project(LedBlinkTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories for STM32 and CppUTest
include_directories(src)

# Define an option to enable or disable the 'tests' directory
option(ENABLE_TESTS "Enable building tests" OFF)

# Conditional build based on ENABLE_TESTS
if(ENABLE_TESTS)
    # Add test directory if tests are enabled
    add_subdirectory(tests)
else()
    # Project name and language
    project(ledblink LANGUAGES C)
    
    # Set build type to Debug if not specified
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    
     #STM32 specific configuration
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -O2 -g -Wall")
    set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_SOURCE_DIR}/src/STM32F446RETX_FLASH.ld -Wl,--gc-sections")
    add_definitions(-DSTM32F446xx)
    message(STATUS "Configured for STM32 hardware.")
    
    # Source files
    set(SOURCES
        ../src/main.c
    )
    
    # Create executable and set include directories
    add_executable(ledblink ${SOURCES})
    target_include_directories(ledblink PRIVATE ../include)
    
     message(STATUS "Project configured for LED blink application.")
endif()
