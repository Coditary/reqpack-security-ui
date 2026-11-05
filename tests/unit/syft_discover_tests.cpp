#include <catch2/catch.hpp>

#include "../support/fake_process_runner.hpp"

#include "rqps/syft_discover_service.h"

#include <filesystem>

TEST_CASE("syft_discover_summary uses injected process output", "[unit][syft_discover]") {
    const std::filesystem::path project = std::filesystem::current_path();

    SyftDiscoverOptions options;
    options.enabled = true;
    options.enforceAllowedRoots = false;

    rqps::test::FakeProcessRunner runner;
    runner.set_response(rqps::test::FakeProcessRunner::make_key("syft", {"version"}),
                        rqps::ProcessOutput{"syft 1.0\n", 0});
    runner.set_response(
        rqps::test::FakeProcessRunner::make_key("syft", {"dir:" + project.string(), "-o", "json", "-q"}),
        rqps::ProcessOutput{
            R"({"source":{"type":"directory"},"artifacts":[{"name":"left-pad","version":"1.0.0","type":"npm"}]})",
            0});

    const rqps::Result<rqps::DiscoverSummary> summary = syft_discover_summary(options, project, runner);
    REQUIRE(summary.has_value());
    REQUIRE(summary.value().packageCount == 1);
    REQUIRE(summary.value().inventory.packages.front().name == "left-pad");
}
