set(CMAKE_CXX_STANDARD "20" CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# Options
option(CHIP_8_ENABLE_TESTS "Enable tests for current build" ON)

# Include modules
include(${CMAKE_CURRENT_LIST_DIR}/clang-tidy.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sanitizers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/options-log.cmake)

# Compiler Flags
if(MSVC)
    add_compile_options(
        /W4          # Serious warnings
        /WX          # Warnings as errors
        /permissive- # Standards conformance
        /sdl         # Security Development Lifecycle checks
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wunused
        -Wcast-align
        -Wconversion
        -Wsign-conversion
        -Wdouble-promotion
        -Wold-style-cast
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wimplicit-fallthrough
        -Wformat=2
        -Wmissing-variable-declarations
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(
            -Wshadow-all 
            -Wreserved-identifier
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wshadow 
            -Wlogical-op 
            -Wduplicated-cond 
            -Wduplicated-branches
            -Wmissing-declarations
        )
    endif()
endif()

# Finalize log
chip_8_log_options()