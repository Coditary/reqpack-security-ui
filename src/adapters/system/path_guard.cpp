#include "rqps/adapters/system/path_guard.hpp"

#include <filesystem>

namespace rqps {

namespace {

bool is_path_under_root(const std::filesystem::path& canonicalPath, const std::filesystem::path& root) {
    std::error_code error;
    const std::filesystem::path canonicalRoot = std::filesystem::weakly_canonical(root, error);
    if (error) {
        return false;
    }
    const std::string target = canonicalPath.string();
    const std::string rootString = canonicalRoot.string();
    if (target == rootString) {
        return true;
    }
    if (target.size() > rootString.size() && target.compare(0, rootString.size(), rootString) == 0 &&
        target[rootString.size()] == '/') {
        return true;
    }
    return false;
}

}  // namespace

Result<std::filesystem::path> PathGuard::canonicalDirectory(const std::filesystem::path& path) {
    std::error_code error;
    const std::filesystem::path absolute = std::filesystem::absolute(path, error);
    if (error) {
        return Result<std::filesystem::path>::fail(
            RqpsError::make(ErrorCode::PathInvalid, "path could not be resolved", path.string()));
    }
    if (!std::filesystem::exists(absolute, error) || !std::filesystem::is_directory(absolute, error)) {
        return Result<std::filesystem::path>::fail(
            RqpsError::make(ErrorCode::PathInvalid, "path is not an existing directory", absolute.string()));
    }
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(absolute, error);
    if (error) {
        return Result<std::filesystem::path>::fail(
            RqpsError::make(ErrorCode::PathInvalid, "path could not be canonicalized", absolute.string()));
    }
    return Result<std::filesystem::path>::ok(canonical);
}

Result<std::filesystem::path> PathGuard::validateDiscoverPath(const SyftDiscoverOptions& options,
                                                              const std::filesystem::path& requestedPath) {
    if (!options.enabled) {
        return Result<std::filesystem::path>::fail(
            RqpsError::make(ErrorCode::DiscoveryDisabled, "repo discovery is disabled in config"));
    }

    const Result<std::filesystem::path> canonical = canonicalDirectory(requestedPath);
    if (!canonical.has_value()) {
        return canonical;
    }

    if (!options.enforceAllowedRoots) {
        return canonical;
    }
    if (options.allowedRoots.empty()) {
        return Result<std::filesystem::path>::fail(
            RqpsError::make(ErrorCode::PathNotAllowed, "path is outside discoverAllowedRoots",
                            canonical.value().string()));
    }

    for (const std::filesystem::path& root : options.allowedRoots) {
        if (is_path_under_root(canonical.value(), root)) {
            return canonical;
        }
    }

    return Result<std::filesystem::path>::fail(
        RqpsError::make(ErrorCode::PathNotAllowed, "path is outside discoverAllowedRoots", canonical.value().string()));
}

bool path_allowed_for_discover(const SyftDiscoverOptions& options, const std::filesystem::path& canonicalPath) {
    if (!options.enforceAllowedRoots) {
        return true;
    }
    if (options.allowedRoots.empty()) {
        return false;
    }
    for (const std::filesystem::path& root : options.allowedRoots) {
        if (is_path_under_root(canonicalPath, root)) {
            return true;
        }
    }
    return false;
}

}  // namespace rqps

bool is_path_allowed_for_discover(const SyftDiscoverOptions& options, const std::filesystem::path& canonicalPath) {
    return rqps::path_allowed_for_discover(options, canonicalPath);
}
