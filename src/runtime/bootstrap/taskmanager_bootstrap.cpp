#include <bootstrap/taskmanager_bootstrap.h>
#include <bootstrap/daemon_bootstrap.h>
#include <runtime_context.h>
#include <run.h>

namespace lute::runtime::bootstrap {

using RuntimeContext = lute::runtime::RuntimeContext;
using AppConfig = lute::runtime::config::AppConfig;

int run_taskmanager(int argc, char** argv) {
    return run_daemon(argc, argv,
        [](RuntimeContext& ctx, const AppConfig& cfg) {
            return lute::runtime::run_taskmanager(ctx, cfg);
        }
    );
}

} // lute::runtime::bootstrap