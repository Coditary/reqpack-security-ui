#include <catch2/catch.hpp>

#include "rqps/domain/error.hpp"

TEST_CASE("Result ok and error states", "[unit][domain][error]") {
    rqps::Result<int> ok = rqps::Result<int>::ok(42);
    REQUIRE(ok.has_value());
    REQUIRE(ok.value() == 42);

    rqps::Result<int> failed = rqps::Result<int>::fail(
        rqps::RqpsError::make(rqps::ErrorCode::PathNotAllowed, "denied", "/tmp/x"));
    REQUIRE_FALSE(failed.has_value());
    REQUIRE(failed.error().code == rqps::ErrorCode::PathNotAllowed);
    REQUIRE(failed.error().message == "denied");
}

TEST_CASE("ErrorCode string mapping", "[unit][domain][error]") {
    REQUIRE(std::string(rqps::to_string(rqps::ErrorCode::ToolNotFound)) == "tool_not_found");
}
