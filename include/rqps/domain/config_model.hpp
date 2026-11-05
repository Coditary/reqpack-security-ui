#pragma once

#include "rqp/security/security_settings.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct SyftDiscoverOptions {
    std::string syftExecutable{"syft"};
    std::vector<std::filesystem::path> allowedRoots;
    bool enabled{true};
    bool enforceAllowedRoots{true};
    std::size_t maxPackages{8000};
};

struct RqpsConfig {
    SecuritySettings settings = default_security_settings();
    SyftDiscoverOptions discover;
    std::filesystem::path globalConfigPath;
    std::optional<std::filesystem::path> projectConfigPath;
};
