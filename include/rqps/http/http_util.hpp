#pragma once

#include <string>

namespace rqps::http {

std::string read_body_field(const std::string& body, const std::string& key);

}  // namespace rqps::http
