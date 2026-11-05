#pragma once

#include "rqps/http/http_server_context.hpp"

namespace httplib {
class Server;
}

namespace rqps::http {

void register_rqps_routes(httplib::Server& server, const HttpServerContext& context);

}  // namespace rqps::http
