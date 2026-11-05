#include "rqps/http/routes/health_routes.hpp"

#include <httplib.h>

namespace rqps::http {

void register_health_routes(httplib::Server& server) {
    server.Get("/api/v1/health", [](const httplib::Request&, httplib::Response& response) {
        response.set_content("{\"ok\":true}", "application/json");
    });
}

}  // namespace rqps::http
