#include "rqps/rqps_cli.h"

#include "rqps/cli/cli_context.h"
#include "rqps/cli/commands/db_command.h"
#include "rqps/cli/commands/discover_command.h"
#include "rqps/cli/commands/doctor_command.h"
#include "rqps/cli/commands/misc_commands.h"
#include "rqps/cli/commands/sbom_command.h"
#include "rqps/cli/commands/scan_command.h"
#include "rqps/cli/commands/test_command.h"
#include "rqps/cli_util.h"

#include <iostream>
#include <string>
#include <vector>

int rqps_cli_main(int argc, char* argv[]) {
    using namespace rqps::cli;

    if (argc < 2) {
        return cmd_test_default();
    }

    const std::string command = argv[1];
    if (command == "--help" || command == "-h" || command == "help") {
        print_usage();
        return 0;
    }

    if (!is_rqps_known_command(command) && !starts_with(command, "-")) {
        std::vector<std::string> args = {"test", command};
        for (int index = 2; index < argc; ++index) {
            args.emplace_back(argv[index]);
        }
        return cmd_test_with_args(std::move(args), false);
    }

    if (command == "test") {
        return cmd_test(argc, argv, false);
    }
    if (command == "audit") {
        return cmd_test(argc, argv, true);
    }
    if (command == "scan") {
        return cmd_scan(argc, argv);
    }
    if (command == "discover") {
        return cmd_discover(argc, argv);
    }
    if (command == "doctor") {
        return cmd_doctor(argc, argv);
    }
    if (command == "init") {
        return cmd_init();
    }
    if (command == "version" || command == "--version") {
        return cmd_version();
    }
    if (command == "status") {
        return cmd_status(argc, argv);
    }
    if (command == "db") {
        return cmd_db(argc, argv);
    }
    if (command == "policy") {
        if (argc >= 3 && std::string(argv[2]) == "show") {
            return cmd_policy_show();
        }
        std::cerr << "usage: rqps policy show\n";
        return 2;
    }
    if (command == "sbom") {
        return cmd_sbom(argc, argv);
    }
    if (command == "remote") {
#ifdef RQPS_HAVE_HTTP
        return cmd_remote(argc, argv);
#else
        return cmd_http_unavailable();
#endif
    }
    if (command == "ping" || command == "health") {
#ifdef RQPS_HAVE_HTTP
        return cmd_ping(argc, argv);
#else
        return cmd_http_unavailable();
#endif
    }
    if (command == "serve") {
#ifdef RQPS_HAVE_HTTP
        return cmd_serve(argc, argv);
#else
        return cmd_http_unavailable();
#endif
    }

    std::cerr << "unknown command: " << command << '\n';
    print_usage();
    return 2;
}
