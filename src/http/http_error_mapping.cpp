#include "rqps/http/http_error_mapping.hpp"

namespace rqps {

int http_status_for_error(const RqpsError& error) {
    switch (error.code) {
        case ErrorCode::PathNotAllowed:
            return 403;
        case ErrorCode::ToolNotFound:
            return 503;
        case ErrorCode::InvalidJson:
        case ErrorCode::InvalidInventoryJson:
        case ErrorCode::PathInvalid:
        case ErrorCode::DiscoveryDisabled:
        case ErrorCode::NoPackages:
            return 400;
        case ErrorCode::ToolFailed:
        case ErrorCode::ScanFailed:
        case ErrorCode::DatabaseSyncFailed:
        case ErrorCode::DatabaseOutdated:
        case ErrorCode::Internal:
        default:
            return 500;
    }
}

}  // namespace rqps
