#include "rqps/cli/commands/discover_command.h"

#include "rqps/cli/cli_context.h"
#include "rqps/syft_discover_service.h"

#include <iostream>

namespace rqps::cli {

int cmd_discover(int argc, char* argv[]) {
    std::vector<std::string> args = argv_to_args(argc, argv);
    const GlobalOptions global = parse_global_pre_args(args);
    std::filesystem::path projectPath = ".";
    if (args.size() > 1) {
        projectPath = args[1];
    }

    RqpsConfig config = load_config_for_path(global, projectPath);
    const SyftDiscoverResult discovered = run_syft_discover(config.discover, projectPath);
    if (discovered.error.has_value()) {
        std::cerr << discovered.error.value() << '\n';
        return 2;
    }
    std::cout << discovered.json << '\n';
    return 0;
}

}  // namespace rqps::cli
