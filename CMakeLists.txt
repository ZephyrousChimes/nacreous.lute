cmake_minimum_required(VERSION 3.25)
project(murli)

# ---- Language Policies ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- Project Options ----
include(cmake/options.cmake)
include(cmake/warnings.cmake)

if (ENABLE_SANITIZERS)
    include(cmake/sanitizers.cmake)
endif()


# ---- Subsystems ----
add_subdirectory(src/core)
add_subdirectory(src/runtime)
add_subdirectory(src/app)
add_subdirectory(src/executors)
add_subdirectory(src/taskmanager)

add_subdirectory(tests)

add_subdirectory(third_party/nacreous_rosette)