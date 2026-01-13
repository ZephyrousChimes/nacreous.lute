# TODO: Change target to STATIC
# TODO: Change target_include_directories to PUBLIC
# TODO: Change target_compile_definitions to PUBLIC

add_library(core INTERFACE
    
)

target_include_directories(core
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_warnings(core)

target_compile_definitions(core
    INTERFACE
        $<$<BOOL:${ENABLE_ASSERTS}>:CORE_ASSERTS_ENABLED>
)