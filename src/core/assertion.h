#pragma once

#include <cstdlib>

#if defined(CORE_ASSERTS_ENABLED)
    #define CORE_ASSERT_IMPL(expr, ...) \
        do { if (!(expr)) std::abort(); } while (false)

    #define CORE_ASSERT(...) CORE_ASSERT_IMPL(__VA_ARGS__)
#else
    #define CORE_ASSERT(...) ((void)(0))
#endif