option(CHIP_8_ENABLE_SANITIZERS "Enable Address and Undefined Behavior sanitizers" OFF)

if(CHIP_8_ENABLE_SANITIZERS)
    if(MSVC)
        # MSVC Address Sanitizer
        # Note: Requires specific VS components installed
        add_compile_options("/fsanitize=address")
        
        # Incremental linking is incompatible with ASan
        add_link_options("/INCREMENTAL:NO")
        
        message(STATUS "MSVC Address Sanitizer enabled (Incremental linking disabled)")
    else()
        # Clang/GCC Sanitizers
        set(SANITIZER_FLAGS "-fsanitize=address,leak,undefined" "-fno-omit-frame-pointer")
        add_compile_options(${SANITIZER_FLAGS})
        add_link_options(${SANITIZER_FLAGS})
        message(STATUS "Sanitizers enabled: ${SANITIZER_FLAGS}")
    endif()
endif()
