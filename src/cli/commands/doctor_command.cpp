#include "rqps/cli/commands/doctor_command.h"

#include "rqps/application/doctor_workflow.hpp"
#include "rqps/cli/cli_context.h"
#include "rqps/cli_util.h"
#include "rqps/ports/i_process_runner.hpp"
#include "rqps/rqps_config.h"
#include "rqps/syft_discover_service.h"

#include <iostream>

namespace rqps::cli {

int cmd_doctor(int argc, char* argv[]) {
    bool fix = false;
    for (int index = 2; index < argc; ++index) {
        if (std::string(argv[index]) == "--fix") {
            fix = true;
        }
    }

    RqpsConfig config = load_rqps_config();
    const rqps::DoctorReport report = rqps::build_doctor_report(config, rqps::default_process_runner());
    bool ok = !report.hasBlockingIssues;

    if (report.syftOk) {
        std::cout << cli_green("syft: ok") << " (" << report.syftPath << ")\n";
    } else {
        std::cout << cli_red("syft: missing") << '\n';
        print_syft_missing_help(report.syftPath);
    }

    if (fix) {
        std::error_code error;
        std::filesystem::create_directories(config.settings.security.osvDatabasePath, error);
        std::filesystem::create_directories(rqps_default_config_dir(), error);
        std::cout << "created data/config directories if needed\n";
        std::cout << "next: rqps db sync\n";
    }

    for (const std::string& line : report.lines) {
        std::cout << line << '\n';
    }
    for (const std::string& warning : report.warnings) {
        std::cout << "warning: " << warning << '\n';
    }

    std::cout << "config: " << config.globalConfigPath << '\n';
    if (config.projectConfigPath.has_value()) {
        std::cout << "project: " << config.projectConfigPath.value() << '\n';
    }

    return ok ? 0 : 1;
}

}  // namespace rqps::cli
