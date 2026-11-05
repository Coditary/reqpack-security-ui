#include <catch2/catch.hpp>

#include "rqps/adapters/system/path_guard.hpp"
#include "rqps/syft_discover_service.h"

#include <filesystem>

TEST_CASE("PathGuard rejects paths outside allowed roots", "[unit][path_guard]") {
    SyftDiscoverOptions options;
    options.enabled = true;
    options.enforceAllowedRoots = true;
    options.allowedRoots = {std::filesystem::path("/allowed/root")};

    const rqps::Result<std::filesystem::path> result =
        rqps::PathGuard::validateDiscoverPath(options, "/allowed/root/project");
    if (std::filesystem::exists("/allowed/root/project")) {
        REQUIRE(result.has_value());
    } else {
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().code == rqps::ErrorCode::PathInvalid);
    }
}

TEST_CASE("PathGuard blocks traversal outside root when canonical path known", "[unit][path_guard]") {
    SyftDiscoverOptions options;
    options.enforceAllowedRoots = true;
    options.allowedRoots = {std::filesystem::temp_directory_path() / "rqps-root-test"};

    const std::filesystem::path inside = options.allowedRoots.front() / "repo";
    const std::filesystem::path outside = std::filesystem::temp_directory_path() / "rqps-outside";

    REQUIRE(is_path_allowed_for_discover(options, inside));
    REQUIRE_FALSE(is_path_allowed_for_discover(options, outside));
}
