#include "rqps/cli/commands/scan_command.h"

#include "rqps/application/policy_evaluator.hpp"
#include "rqps/application/service_registry.hpp"
#include "rqps/cli/cli_context.h"
#include "rqps/cli/scan_output.h"
#include "rqps/inventory_json.h"
#include "rqps/ports/i_scanner.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace rqps::cli {

int cmd_scan(int argc, char* argv[]) {
    std::vector<std::string> args = argv_to_args(argc, argv);
    const GlobalOptions global = parse_global_pre_args(args);
    if (args.size() < 2 || args[1] != "-f" || args.size() < 3) {
        std::cerr << "usage: rqps scan -f inventory.json\n";
        return 2;
    }

    RqpsConfig config = load_rqps_config(global.configPath);
    if (!ensure_db_if_needed(config, global.noSync, global.quiet)) {
        return 2;
    }

    std::ifstream input(args[2]);
    std::ostringstream buffer;
    buffer << input.rdbuf();

    std::string parseError;
    const Graph graph = graph_from_inventory_json(buffer.str(), parseError);
    if (!parseError.empty()) {
        std::cerr << parseError << '\n';
        return 2;
    }

    const Inventory inventory = rqps::inventory_from_graph(graph);
    const rqps::Result<std::vector<ValidationFinding>> scanned =
        rqps::default_scanner().scanInventory(config.settings, inventory);
    if (!scanned.has_value()) {
        std::cerr << scanned.error().message << '\n';
        return 2;
    }

    ScanOutputOptions outputOptions;
    outputOptions.json = global.json;
    outputOptions.quiet = global.quiet;
    const rqps::PolicyEvaluation policy =
        rqps::evaluate_policy(config.settings, scanned.value(), outputOptions.failOnFindings);
    return print_policy_evaluation(policy, outputOptions);
}

}  // namespace rqps::cli
