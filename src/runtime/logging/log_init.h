#pragma once

#include <config/config.h>
#include <bootstrap/runtime_mode.h>

namespace lute::runtime::logging {

using AppConfig = lute::runtime::config::AppConfig;
using RuntimeMode = lute::runtime::bootstrap::RuntimeMode;

void init_logging(const AppConfig& config, RuntimeMode);

} // lute::runtime::logging