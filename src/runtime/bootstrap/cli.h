#pragma once

#include <bootstrap/runtime_mode.h>
#include <string>

namespace lute::runtime::bootstrap {

struct CliOptions {
    RuntimeMode mode;
    std::string config_path;
};

CliOptions parse_cli(int argc, char** argv);

} // namespace lute::runtime::bootstrap