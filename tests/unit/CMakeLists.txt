add_executable(core_tests
    taskmanager/channels/InMemoryChannelTest.cpp
)

target_link_libraries(core_tests 
    PRIVATE 
        core
        dataplane
        GTest::gtest_main
)

target_include_directories(core_tests
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

gtest_discover_tests(core_tests)
enable_warnings(core_tests)