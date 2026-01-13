#pragma once

#include <bootstrap/runtime_mode.h>

namespace lute::runtime {

struct RuntimeContext {
    lute::runtime::bootstrap::RuntimeMode mode;
};

} // lute::runtime