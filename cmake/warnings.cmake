function(enable_warnings target)
    get_target_property(type ${target} TYPE)

    if (type STREQUAL "INTERFACE_LIBRARY")
        set(scope INTERFACE)
    else()
        set(scope PRIVATE)
    endif()

    target_compile_options(${target} ${scope}
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
    
endfunction()