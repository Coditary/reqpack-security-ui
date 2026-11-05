#pragma once

#include <rqp/security/json_util.h>

inline std::string json_escape(const std::string& value) {
    return rqp_json_escape(value);
}
