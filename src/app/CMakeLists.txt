add_executable(taskmanager
    taskmanager_main.cpp
)

foreach(app taskmanager)
    target_link_libraries(${app}
        PRIVATE runtime core
    )
    enable_warnings(${app})
endforeach()