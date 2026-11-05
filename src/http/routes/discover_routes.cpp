#include "rqps/http/routes/discover_routes.hpp"

#include "rqps/http/http_error_mapping.hpp"
#include "rqps/http/http_util.hpp"
#include "rqps/json_util.h"
#include "rqps/syft_discover_service.h"

#include <httplib.h>

namespace rqps::http {

void register_discover_routes(httplib::Server& server, const HttpServerContext& context) {
    server.Get("/api/v1/discover/info", [&context](const httplib::Request&, httplib::Response& response) {
        response.set_content(syft_discover_info_json(context.config.discover, context.processRunner), "application/json");
    });

    server.Post("/api/v1/discover", [&context](const httplib::Request& request, httplib::Response& response) {
        const std::string path = read_body_field(request.body, "path");
        if (path.empty()) {
            response.status = 400;
            response.set_content("{\"error\":\"missing path field\"}", "application/json");
            return;
        }
        const rqps::Result<rqps::DiscoverSummary> discover =
            syft_discover_summary(context.config.discover, path, context.processRunner);
        if (!discover.has_value()) {
            response.status = http_status_for_error(discover.error());
            response.set_content("{\"error\":\"" + json_escape(discover.error().message) + "\"}", "application/json");
            return;
        }
        response.set_content(discover_summary_to_json(discover.value()), "application/json");
    });
}

}  // namespace rqps::http
