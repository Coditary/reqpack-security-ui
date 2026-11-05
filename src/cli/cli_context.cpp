#include "rqps/cli/cli_context.h"

#include "rqps/cli_util.h"
#include "rqps/db_service.h"
#include "rqps/rqps_config.h"

#include "rqp/security/vulnerability_database.h"

#include <iostream>
#include <vector>

namespace rqps::cli {

bool starts_with(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

GlobalOptions parse_global_pre_args(std::vector<std::string>& args) {
    GlobalOptions options;
    std::vector<std::string> kept;
    for (std::size_t index = 0; index < args.size(); ++index) {
        const std::string& arg = args[index];
        if (arg == "--json") {
            options.json = true;
            continue;
        }
        if (arg == "--quiet") {
            options.quiet = true;
            continue;
        }
        if (arg == "--no-sync" || arg == "--offline") {
            options.noSync = true;
            continue;
        }
        if (arg == "--config" && index + 1 < args.size()) {
            options.configPath = args[++index];
            continue;
        }
        kept.push_back(arg);
    }
    args = std::move(kept);
    return options;
}

RqpsConfig load_config_for_path(const GlobalOptions& global, const std::filesystem::path& projectPath) {
    RqpsConfig config = load_rqps_config(global.configPath);
    config.discover.enforceAllowedRoots = false;
    return merge_project_config(config, projectPath);
}

bool ensure_db_if_needed(RqpsConfig& config, bool noSync, bool quiet) {
    VulnerabilityDatabase database(config.settings);
    database.ensureReady();
    if (database.hasAdvisories()) {
        return true;
    }
    if (noSync) {
        print_db_empty_help();
        return false;
    }
    if (!quiet) {
        std::cerr << cli_yellow("Updating vulnerability database (first run or empty DB)…") << '\n';
    }
    const auto findings = rqps_db_sync(config.settings, rqps_default_sync_ecosystems());
    for (const ValidationFinding& finding : findings) {
        if (finding.kind == "sync_error") {
            std::cerr << "sync error: " << finding.message << '\n';
            return false;
        }
    }
    database.ensureReady();
    if (!database.hasAdvisories()) {
        std::cerr << "error: database still empty after sync\n";
        return false;
    }
    return true;
}

std::vector<std::string> argv_to_args(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }
    return args;
}

void print_usage() {
    print_rqps_banner_help();
    std::cerr
        << "Usage: rqps [command] [options]\n"
        << "       rqps              run test in current directory\n"
        << "       rqps <path>       run test in <path>\n\n"
        << "Commands:\n"
        << "  test [path]           Discover (syft) + vulnerability scan (default: .)\n"
        << "  audit [path]          Same as test with audit-oriented defaults\n"
        << "  scan -f <inventory.json>\n"
        << "  discover [path]       Syft catalog only\n"
        << "  doctor [--fix]        Local toolchain + DB readiness\n"
        << "  init                  Create .rqps.json in current directory\n"
        << "  version               Show rqps version\n"
        << "  status                Database and policy summary\n"
        << "  db sync [--ecosystem E]...\n"
        << "  db status\n"
        << "  sbom [path] [-o file] [--format json|cyclonedx-json]\n"
        << "  sbom merge <a.json> <b.json> [-o merged.json]\n"
        << "  policy show\n";
#ifdef RQPS_HAVE_HTTP
    std::cerr
        << "  remote add <name> <url>\n"
        << "  remote list | remove <name> | use <name>\n"
        << "  ping [--remote <name>]\n"
        << "  serve [--config path] [--host] [--port]\n";
#endif
    std::cerr << "Global: --config <path>  --json  --quiet  --no-sync  --offline\n"
              << "Try: rqps test -h\n";
}

}  // namespace rqps::cli
