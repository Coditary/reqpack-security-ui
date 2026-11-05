#pragma once

#include <string>
#include <utility>
#include <variant>

namespace rqps {

enum class ErrorCode {
    Ok = 0,
    ToolNotFound,
    ToolFailed,
    PathNotAllowed,
    PathInvalid,
    DiscoveryDisabled,
    InvalidInventoryJson,
    DatabaseOutdated,
    DatabaseSyncFailed,
    ScanFailed,
    NoPackages,
    InvalidJson,
    Internal,
};

inline const char* to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::Ok:
            return "ok";
        case ErrorCode::ToolNotFound:
            return "tool_not_found";
        case ErrorCode::ToolFailed:
            return "tool_failed";
        case ErrorCode::PathNotAllowed:
            return "path_not_allowed";
        case ErrorCode::PathInvalid:
            return "path_invalid";
        case ErrorCode::DiscoveryDisabled:
            return "discovery_disabled";
        case ErrorCode::InvalidInventoryJson:
            return "invalid_inventory_json";
        case ErrorCode::DatabaseOutdated:
            return "database_outdated";
        case ErrorCode::DatabaseSyncFailed:
            return "database_sync_failed";
        case ErrorCode::ScanFailed:
            return "scan_failed";
        case ErrorCode::NoPackages:
            return "no_packages";
        case ErrorCode::InvalidJson:
            return "invalid_json";
        case ErrorCode::Internal:
            return "internal";
    }
    return "unknown";
}

struct RqpsError {
    ErrorCode code{ErrorCode::Internal};
    std::string message;
    std::string context;
    int exitCode{-1};

    static RqpsError make(ErrorCode code, std::string message, std::string context = {}, int processExit = -1) {
        return RqpsError{code, std::move(message), std::move(context), processExit};
    }
};

template <typename T>
class Result {
public:
    Result(T value) : storage_(std::move(value)) {}  // NOLINT(google-explicit-constructor)
    Result(RqpsError error) : storage_(std::move(error)) {}  // NOLINT(google-explicit-constructor)

    [[nodiscard]] bool has_value() const { return std::holds_alternative<T>(storage_); }
    [[nodiscard]] explicit operator bool() const { return has_value(); }

    T& value() & { return std::get<T>(storage_); }
    const T& value() const& { return std::get<T>(storage_); }
    T&& value() && { return std::get<T>(std::move(storage_)); }

    const RqpsError& error() const { return std::get<RqpsError>(storage_); }

    static Result ok(T value) { return Result{std::move(value)}; }
    static Result fail(RqpsError error) { return Result{std::move(error)}; }

private:
    std::variant<T, RqpsError> storage_;
};

}  // namespace rqps
