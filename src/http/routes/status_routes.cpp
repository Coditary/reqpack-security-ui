#include "rqps/http/routes/status_routes.hpp"

#include "rqps/local_status.h"

#include <httplib.h>

namespace rqps::http {

void register_status_routes(httplib::Server& server, const RqpsConfig& config) {
    server.Get("/api/v1/status", [&config](const httplib::Request&, httplib::Response& response) {
        response.set_content(local_status_to_json(build_local_status(config.settings)), "application/json");
    });
}

}  // namespace rqps::http
