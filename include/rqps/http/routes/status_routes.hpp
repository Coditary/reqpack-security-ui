#pragma once

#include "rqps/domain/config_model.hpp"

namespace httplib {
class Server;
}

namespace rqps::http {

void register_status_routes(httplib::Server& server, const RqpsConfig& config);

}  // namespace rqps::http
