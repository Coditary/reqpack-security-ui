#include <catch2/catch.hpp>

#include "rqps/json_util.h"

TEST_CASE("json_escape quotes and control characters", "[unit][json_util]") {
    CHECK(json_escape("plain") == "plain");
    CHECK(json_escape("a\"b") == "a\\\"b");
    CHECK(json_escape("line\nbreak") == "line\\nbreak");
    CHECK(json_escape("tab\there") == "tab\\there");
}
