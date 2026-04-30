cmake_minimum_required(VERSION 3.27)

project(
    chip-8-test
    LANGUAGES CXX
)

include(CTest)

add_executable(${PROJECT_NAME}
    test.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        GTest::gtest
        GTest::gtest_main
        chip-8::headers
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})