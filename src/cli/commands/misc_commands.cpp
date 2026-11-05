#include "rqps/cli/commands/misc_commands.h"

#include "rqps/cli/cli_context.h"
#include "rqps/rqps_config.h"
#include "rqps/syft_discover_service.h"

#ifdef RQPS_HAVE_HTTP
#include "rqps/http/serve_runner.h"
#include "rqps/remote_config.h"
#endif

#include "rqp/security/security_config.h"

#include <fstream>
#include <iostream>

namespace rqps::cli {

int cmd_init() {
    const std::filesystem::path target = std::filesystem::current_path() / ".rqps.json";
    if (std::filesystem::exists(target)) {
        std::cerr << ".rqps.json already exists — not overwriting\n";
        return 1;
    }
    std::ofstream output(target);
    output << R"({
  "severityThreshold": "low",
  "ignoreVulnerabilityIds": []
}
)";
    std::cout << "wrote " << target << '\n';
    std::cout << "edit ignoreVulnerabilityIds or thresholds, then run: rqps test\n";
    return 0;
}

int cmd_version() {
#ifdef RQPS_VERSION
    std::cout << "rqps " << RQPS_VERSION << '\n';
#else
    std::cout << "rqps (dev build)\n";
#endif
    RqpsConfig config = load_rqps_config();
    std::cout << "osvDatabasePath: " << config.settings.security.osvDatabasePath << '\n';
    std::cout << "syft: " << (syft_is_available(config.discover.syftExecutable) ? "ok" : "missing") << '\n';
    return 0;
}

int cmd_policy_show() {
    RqpsConfig config = load_rqps_config();
    std::cout << "severityThreshold: " << to_string(config.settings.security.severityThreshold) << '\n';
    std::cout << "scoreThreshold: " << config.settings.security.scoreThreshold << '\n';
    std::cout << "onUnsafe: " << to_string(config.settings.security.onUnsafe) << '\n';
    std::cout << "strictEcosystemMapping: " << (config.settings.security.strictEcosystemMapping ? "yes" : "no") << '\n';
    std::cout << "ignoreVulnerabilityIds:\n";
    for (const std::string& id : config.settings.security.ignoreVulnerabilityIds) {
        std::cout << "  - " << id << '\n';
    }
    std::cout << "allowVulnerabilityIds:\n";
    for (const std::string& id : config.settings.security.allowVulnerabilityIds) {
        std::cout << "  - " << id << '\n';
    }
    return 0;
}

int cmd_http_unavailable() {
    std::cerr << "HTTP support not built (RQPS_BUILD_HTTP=OFF). Use rqps-ui or rebuild with -DRQPS_BUILD_HTTP=ON.\n";
    return 2;
}

#ifdef RQPS_HAVE_HTTP
int cmd_remote(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "usage: rqps remote add|list|remove|use\n";
        return 2;
    }
    RemoteStore store = load_remote_store();
    const std::string sub = argv[2];
    if (sub == "list") {
        std::cout << "default: " << (store.defaultRemote.empty() ? "(none)" : store.defaultRemote) << '\n';
        for (const auto& [name, url] : store.remotes) {
            std::cout << name << " -> " << url << '\n';
        }
        return 0;
    }
    if (sub == "add" && argc >= 5) {
        store.remotes[argv[3]] = argv[4];
        if (store.defaultRemote.empty()) {
            store.defaultRemote = argv[3];
        }
        save_remote_store(store);
        return 0;
    }
    if (sub == "remove" && argc >= 4) {
        store.remotes.erase(argv[3]);
        if (store.defaultRemote == argv[3]) {
            store.defaultRemote.clear();
        }
        save_remote_store(store);
        return 0;
    }
    if (sub == "use" && argc >= 4) {
        if (!store.remotes.contains(argv[3])) {
            std::cerr << "unknown remote\n";
            return 2;
        }
        store.defaultRemote = argv[3];
        save_remote_store(store);
        return 0;
    }
    return 2;
}

int cmd_ping(int argc, char* argv[]) {
    RemoteStore store = load_remote_store();
    std::string remoteName = store.defaultRemote;
    for (int index = 2; index < argc; ++index) {
        if (std::string(argv[index]) == "--remote" && index + 1 < argc) {
            remoteName = argv[++index];
        }
    }
    if (remoteName.empty() || !store.remotes.contains(remoteName)) {
        std::cerr << "no remote configured (rqps remote add …)\n";
        return 2;
    }
    const std::string url = store.remotes[remoteName];
    const auto error = remote_ping(url);
    if (error.has_value()) {
        std::cerr << "ping failed: " << error.value() << '\n';
        return 2;
    }
    std::cout << "ping ok: " << url << '\n';
    return 0;
}

int cmd_serve(int argc, char* argv[]) {
    ServeOptions options;
    for (int index = 2; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--config" && index + 1 < argc) {
            options.configPath = argv[++index];
            continue;
        }
        if (arg == "--host" && index + 1 < argc) {
            options.host = argv[++index];
            continue;
        }
        if (arg == "--port" && index + 1 < argc) {
            options.port = std::stoi(argv[++index]);
            continue;
        }
    }
    return run_rqps_serve(options);
}
#endif

}  // namespace rqps::cli
