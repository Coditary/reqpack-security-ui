#include "rqps/cli/commands/test_command.h"

#include "rqps/application/service_registry.hpp"
#include "rqps/cli/cli_context.h"
#include "rqps/cli/scan_output.h"
#include "rqps/cli_util.h"
#include "rqps/syft_discover_service.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace rqps::cli {

namespace {

struct TestOptions {
    bool dryRun{false};
    std::optional<std::size_t> maxPackages;
};

TestOptions parse_test_options(std::vector<std::string>& args) {
    TestOptions options;
    std::vector<std::string> kept;
    kept.push_back(args.empty() ? "test" : args[0]);
    for (std::size_t index = 1; index < args.size(); ++index) {
        const std::string& arg = args[index];
        if (arg == "--dry-run") {
            options.dryRun = true;
            continue;
        }
        if (arg == "--max-packages" && index + 1 < args.size()) {
            options.maxPackages = static_cast<std::size_t>(std::stoul(args[++index]));
            continue;
        }
        if (arg == "-h" || arg == "--help") {
            print_test_command_help();
            std::exit(0);
        }
        kept.push_back(arg);
    }
    args = std::move(kept);
    return options;
}

int run_test_from_args(std::vector<std::string> args, bool auditMode) {
    const GlobalOptions global = parse_global_pre_args(args);
    const TestOptions testOptions = parse_test_options(args);

    std::filesystem::path projectPath = ".";
    if (args.size() > 1 && !starts_with(args[1], "-")) {
        projectPath = args[1];
    }

    RqpsConfig config = load_config_for_path(global, projectPath);
#if defined(RQPS_UI_HAVE_AUDIT_FAIL_ON_FINDINGS)
    if (auditMode) {
        config.settings.security.auditFailOnFindings = true;
    }
#endif
    if (testOptions.maxPackages.has_value()) {
        config.discover.maxPackages = testOptions.maxPackages.value();
    }

    if (!testOptions.dryRun) {
        if (!ensure_db_if_needed(config, global.noSync, global.quiet)) {
            return 2;
        }
    }

    if (!syft_is_available(config.discover.syftExecutable)) {
        print_syft_missing_help(config.discover.syftExecutable);
        return 2;
    }

    if (!global.quiet) {
        std::cerr << "Cataloging dependencies (syft)…\n";
    }

    rqps::AuditWorkflowOptions workflowOptions;
    workflowOptions.dryRun = testOptions.dryRun;
    workflowOptions.failOnFindings = true;

    const rqps::Result<rqps::AuditReport> audit = rqps::default_audit_workflow().run(config, projectPath, workflowOptions);
    if (!audit.has_value()) {
        std::cerr << "discover error: " << audit.error().message << '\n';
        return 2;
    }

    if (testOptions.dryRun) {
        if (global.json) {
            std::cout << audit.value().discoverJson << '\n';
        } else if (!global.quiet) {
            std::cout << "dry-run: cataloged " << audit.value().discovery->packageCount << " package(s) under "
                      << projectPath << '\n';
            std::cout << "hint: run rqps test" << (projectPath == "." ? "" : (" " + projectPath.string())) << '\n';
        }
        return 0;
    }

    if (!global.quiet) {
        std::cerr << "Scanning…\n";
    }

    ScanOutputOptions outputOptions;
    outputOptions.json = global.json;
    outputOptions.quiet = global.quiet;
    outputOptions.failOnFindings = true;
    return print_policy_evaluation(audit.value().policy, outputOptions);
}

}  // namespace

int cmd_test(int argc, char* argv[], bool auditMode) {
    return run_test_from_args(argv_to_args(argc, argv), auditMode);
}

int cmd_test_with_args(std::vector<std::string> args, bool auditMode) {
    return run_test_from_args(std::move(args), auditMode);
}

int cmd_test_default() {
    return run_test_from_args({"test"}, false);
}

}  // namespace rqps::cli
