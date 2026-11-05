#include <catch2/catch.hpp>

#include "rqps/rqps_config.h"

#include "test_helpers.h"

#include <fstream>

TEST_CASE("load_rqps_config reads explicit JSON file", "[unit][rqps_config]") {
    TempDir tempDir{"rqps-config"};
    const std::filesystem::path configPath = tempDir.path() / "config.json";
    write_file(configPath, R"({
      "enabled": true,
      "osvDatabasePath": "/tmp/rqps-test-db",
      "severityThreshold": "high"
    })");

    const RqpsConfig config = load_rqps_config(configPath);
    CHECK(config.settings.security.enabled);
    CHECK(config.settings.security.osvDatabasePath == "/tmp/rqps-test-db");
    CHECK(config.settings.security.severityThreshold == SeverityLevel::HIGH);
}
