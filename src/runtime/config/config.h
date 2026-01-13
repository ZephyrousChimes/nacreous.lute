#pragma once
#include <string>

namespace lute::runtime::config {

struct AppConfig {
    int worker_threads;
};

AppConfig load_config(const std::string& path);

} // namespace lute::runtime::config