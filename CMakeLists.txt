cmake_minimum_required(VERSION 3.25)

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

add_library(_headers INTERFACE)

target_include_directories(_headers
    INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/include
)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(_headers
    INTERFACE 
        SDL3::SDL3
)

add_library(${PROJECT_NAME}::headers ALIAS _headers)

# Main executable

add_executable(${PROJECT_NAME}
    src/main.cpp
)

find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_NAME}::headers
        cxxopts::cxxopts
)

# Tests setup
if(CHIP_8_ENABLE_TESTS)
    enable_testing()
    
    include(CTest)

    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test/)
endif()
