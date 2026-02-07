option(ENABLE_ASSERTS "Enable runtime assertions" ON)
option(ENABLE_LOGGING "Enable logging" ON)
option(ENABLE_TRACING "Enable tracing" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

add_compile_options(-march=native)