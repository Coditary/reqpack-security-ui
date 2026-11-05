#include "rqps/http/routes/register_routes.hpp"

#include "rqps/http/routes/discover_routes.hpp"
#include "rqps/http/routes/health_routes.hpp"
#include "rqps/http/routes/scan_routes.hpp"
#include "rqps/http/routes/status_routes.hpp"

#include <httplib.h>

namespace rqps::http {

void register_rqps_routes(httplib::Server& server, const HttpServerContext& context) {
    server.set_default_headers({{"Access-Control-Allow-Origin", "*"},
                                {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
                                {"Access-Control-Allow-Headers", "Content-Type"}});

    server.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& response) { response.status = 204; });

    register_health_routes(server);
    register_status_routes(server, context.config);
    register_discover_routes(server, context);
    register_scan_routes(server, context);
}

}  // namespace rqps::http
