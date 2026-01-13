#include <bootstrap/cli.h>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace lute::runtime::bootstrap {

static RuntimeMode parse_mode(const std::string& s) {
    if (s == "prod") return RuntimeMode::Prod;
    if (s == "dev") return RuntimeMode::Dev;
    if (s == "sim") return RuntimeMode::Sim;
    throw std::runtime_error("Invalid --mode value: '" + s + 
                           "'. Expected: prod, dev, or sim");
}

CliOptions parse_cli(int argc, char** argv) {
    if (argc < 3) {
        throw std::runtime_error(
            "Usage: " + std::string(argv[0]) + 
            " --config <path> --mode <prod|dev|sim>");
    }

    CliOptions opts{};
    bool config_provided = false;

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
            throw std::runtime_error("Flag '" + std::string(argv[i]) + 
                                   "' requires a value");
        }

        std::string key = argv[i];
        std::string val = argv[i + 1];

        if (key == "--mode") {
            opts.mode = parse_mode(val);
        } else if (key == "--config") {
            opts.config_path = val;
            config_provided = true;
        } else {
            throw std::runtime_error("Unknown CLI flag: '" + key + "'");
        }
    }

    if (!config_provided) {
        throw std::runtime_error("Required flag --config not provided");
    }

    if (!std::filesystem::exists(opts.config_path)) {
        throw std::runtime_error("Config file does not exist: " + 
                               opts.config_path);
    }

    return opts;
}

}