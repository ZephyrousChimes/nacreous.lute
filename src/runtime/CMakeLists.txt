add_library(runtime STATIC
    run_taskmanager.cpp

    bootstrap/cli.cpp
    bootstrap/taskmanager_bootstrap.cpp

    config/config.cpp

    lifecycle/shutdown_manager.cpp
    lifecycle/signal_install.cpp

    logging/log_frontend.cpp
    logging/log_init.cpp
    logging/backends/null_backend.cpp
)

target_include_directories(runtime
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(runtime
    PUBLIC core
)

enable_warnings(runtime)

target_compile_definitions(runtime
    PRIVATE
        $<$<BOOL:${ENABLE_LOGGING}>:RUNTIME_LOGGING_ENABLED>
)