cmake_minimum_required(VERSION 4.1)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(
    chip-8 
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/build-setup.cmake)

# Headers

add_library(_headers
    src/chip_8/chip_8.cpp
    src/chip_8/instruction_set.cpp
)

target_include_directories(_headers
    PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/include
)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(_headers
    PUBLIC 
        SDL3::SDL3
)

add_library(${PROJECT_NAME}::headers ALIAS _headers)

# Main executable

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_NAME}::headers
)

# Tests setup

option(CHIP_8_ENABLE_TESTS "Enable tests for current build" ON)
message(STATUS "CHIP_8_ENABLE_TESTS: ${CHIP_8_ENABLE_TESTS}")

if(CHIP_8_ENABLE_TESTS)
    enable_testing()
    
    include(CTest)

    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test/)
endif()
