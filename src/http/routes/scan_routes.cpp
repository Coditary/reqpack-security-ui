#include "rqps/http/routes/scan_routes.hpp"

#include "rqps/application/scan_workflow.hpp"
#include "rqps/http/http_error_mapping.hpp"
#include "rqps/http/http_util.hpp"
#include "rqps/json_util.h"

#include <httplib.h>

namespace rqps::http {

void register_scan_routes(httplib::Server& server, const HttpServerContext& context) {
    server.Post("/api/v1/scan", [&context](const httplib::Request& request, httplib::Response& response) {
        const std::string input = read_body_field(request.body, "input");
        if (input.empty()) {
            response.status = 400;
            response.set_content("{\"error\":\"missing input field\"}", "application/json");
            return;
        }
        const rqps::Result<std::string> scan =
            scan_inventory_json_report(context.config.settings, input, context.scanner);
        if (!scan.has_value()) {
            response.status = http_status_for_error(scan.error());
            response.set_content("{\"error\":\"" + json_escape(scan.error().message) + "\"}", "application/json");
            return;
        }
        response.set_content(scan.value(), "application/json");
    });
}

}  // namespace rqps::http
