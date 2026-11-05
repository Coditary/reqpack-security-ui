#include "rqps/cli/commands/db_command.h"

#include "rqps/cli/cli_context.h"
#include "rqps/db_service.h"
#include "rqps/local_status.h"
#include "rqps/rqps_config.h"

#include <iostream>
#include <set>

namespace rqps::cli {

int cmd_status(int argc, char* argv[]) {
    std::vector<std::string> args = argv_to_args(argc, argv);
    const GlobalOptions global = parse_global_pre_args(args);
    RqpsConfig config = load_rqps_config(global.configPath);
    const LocalStatusReport report = build_local_status(config.settings);
    if (global.json) {
        std::cout << local_status_to_json(report) << '\n';
        return 0;
    }
    for (const std::string& line : report.lines) {
        std::cout << line << '\n';
    }
    for (const std::string& warning : report.warnings) {
        std::cout << "warning: " << warning << '\n';
    }
    return report.hasBlockingIssues ? 1 : 0;
}

int cmd_db(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "usage: rqps db sync|status\n";
        return 2;
    }
    std::vector<std::string> args = argv_to_args(argc, argv);
    const GlobalOptions global = parse_global_pre_args(args);
    RqpsConfig config = load_rqps_config(global.configPath);

    if (args[1] == "status") {
        return cmd_status(argc, argv);
    }

    if (args[1] != "sync") {
        std::cerr << "unknown db subcommand\n";
        return 2;
    }

    std::set<std::string> ecosystems = rqps_default_sync_ecosystems();
    for (std::size_t index = 2; index < args.size(); ++index) {
        if (args[index] == "--ecosystem" && index + 1 < args.size()) {
            ecosystems = {args[++index]};
        }
    }

    if (!global.quiet) {
        std::cerr << "Syncing OSV data for " << ecosystems.size() << " ecosystem(s)…\n";
    }
    const auto findings = rqps_db_sync(config.settings, ecosystems);
    for (const ValidationFinding& finding : findings) {
        if (finding.kind == "sync_error") {
            std::cerr << "sync error: " << finding.message << '\n';
            return 2;
        }
    }
    if (!global.quiet) {
        std::cout << "db sync: ok\n";
    }
    return 0;
}

}  // namespace rqps::cli
