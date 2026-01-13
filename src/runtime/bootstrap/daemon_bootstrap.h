#pragma once

#include <bootstrap/cli.h>
#include <config/config.h>
#include <logging/log_init.h>
#include <lifecycle/signal_install.h>
#include <runtime_context.h>

#include <iostream>

namespace lute::runtime::bootstrap {

template <typename RunFn>
int run_daemon(int argc, char** argv, RunFn&& run_fn) noexcept(false) {
    try {
        const CliOptions cli = parse_cli(argc, argv);

        const lute::runtime::config::AppConfig config = 
            lute::runtime::config::load_config(cli.config_path);

        lute::runtime::logging::init_logging(config, cli.mode);

        lute::runtime::lifecycle::setup_signal_handlers();

        lute::runtime::RuntimeContext ctx {
            .mode = cli.mode
        };

        return std::forward<RunFn>(run_fn)(ctx, config);

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }    
}

} // namespace lute::runtime::bootstrap