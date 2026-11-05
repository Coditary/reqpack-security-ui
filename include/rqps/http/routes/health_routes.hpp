#pragma once

namespace httplib {
class Server;
}

namespace rqps::http {

void register_health_routes(httplib::Server& server);

}  // namespace rqps::http
