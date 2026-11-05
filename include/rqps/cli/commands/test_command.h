#pragma once

#include <string>
#include <vector>

namespace rqps::cli {

int cmd_test(int argc, char* argv[], bool auditMode);

int cmd_test_with_args(std::vector<std::string> args, bool auditMode);

int cmd_test_default();

}  // namespace rqps::cli
