#pragma once

#include "rqps/domain/error.hpp"

namespace rqps {

int http_status_for_error(const RqpsError& error);

}  // namespace rqps
