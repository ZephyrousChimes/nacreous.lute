#include <run.h>
#include <runtime_context.h>
#include <config/config.h>
#include <assertion.h>

namespace lute::runtime {

int run_taskmanager(
    lute::runtime::RuntimeContext& ctx, 
    const lute::runtime::config::AppConfig& config
) {
    CORE_ASSERT(1 == 1, "Yep, looks true");
}

} // lute::runtime