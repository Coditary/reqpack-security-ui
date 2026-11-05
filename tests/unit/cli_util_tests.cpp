#include <catch2/catch.hpp>

#include "rqps/cli_util.h"

TEST_CASE("is_rqps_known_command recognizes CLI verbs", "[unit][cli_util]") {
    CHECK(is_rqps_known_command("test"));
    CHECK(is_rqps_known_command("doctor"));
    CHECK(is_rqps_known_command("db"));
    CHECK_FALSE(is_rqps_known_command("not-a-command"));
    CHECK_FALSE(is_rqps_known_command(""));
}
