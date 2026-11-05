#include "rqps/http/serve_runner.h"

#include "rqps/application/service_registry.hpp"
#include "rqps/http/http_server_context.hpp"
#include "rqps/http/routes/register_routes.hpp"
#include "rqps/rqps_config.h"

#include <httplib.h>

#include <filesystem>
#include <iostream>

namespace {

RqpsConfig load_server_config(const std::filesystem::path& configPath) {
    if (!configPath.empty() && std::filesystem::exists(configPath)) {
        return load_rqps_config(configPath);
    }
    return load_rqps_config();
}

}  // namespace

int run_rqps_serve(const ServeOptions& options) {
    RqpsConfig appConfig = load_server_config(options.configPath);
    appConfig.discover.enforceAllowedRoots = true;
    if (appConfig.discover.allowedRoots.empty()) {
        std::cerr << "warning: discoverAllowedRoots is empty; POST /api/v1/discover will reject paths\n";
    }

    rqps::http::HttpServerContext context{appConfig, rqps::default_process_runner(), rqps::default_scanner()};

    httplib::Server server;
    rqps::http::register_rqps_routes(server, context);

    std::cout << "rqps-ui listening on http://" << options.host << ':' << options.port << " (JSON API only)\n";
    if (!server.listen(options.host, options.port)) {
        std::cerr << "failed to bind " << options.host << ':' << options.port << '\n';
        return 1;
    }
    return 0;
}
