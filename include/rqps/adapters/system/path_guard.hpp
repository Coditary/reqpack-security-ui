#pragma once

#include "rqps/domain/config_model.hpp"
#include "rqps/domain/error.hpp"

#include <filesystem>

namespace rqps {

class PathGuard {
public:
    static Result<std::filesystem::path> canonicalDirectory(const std::filesystem::path& path);

    static Result<std::filesystem::path> validateDiscoverPath(const SyftDiscoverOptions& options,
                                                              const std::filesystem::path& requestedPath);
};

}  // namespace rqps
