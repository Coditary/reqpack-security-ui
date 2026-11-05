#include "rqps/http/http_util.hpp"

#include <cctype>

namespace rqps::http {

std::string read_body_field(const std::string& body, const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const std::size_t start = body.find(needle);
    if (start == std::string::npos) {
        return {};
    }
    std::size_t pos = start + needle.size();
    while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos]))) {
        ++pos;
    }
    if (pos >= body.size() || body[pos] != '\"') {
        return {};
    }
    ++pos;
    std::string value;
    while (pos < body.size()) {
        const char character = body[pos++];
        if (character == '\\' && pos < body.size()) {
            value.push_back(body[pos++]);
            continue;
        }
        if (character == '\"') {
            break;
        }
        value.push_back(character);
    }
    return value;
}

}  // namespace rqps::http
