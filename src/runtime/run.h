#pragma once

#include <bootstrap/daemon_bootstrap.h>
#include <bootstrap/runtime_mode.h>
#include <config/config.h>

namespace lute::runtime {

int run_taskmanager(
    lute::runtime::RuntimeContext& ctx, 
    const lute::runtime::config::AppConfig& config
);

} // namespace lute::runtime