#pragma once

#include "rqp/security/security_settings.h"

#include <string>
#include <vector>

struct LocalStatusReport {
    std::vector<std::string> lines;
    std::vector<std::string> warnings;
    bool hasBlockingIssues{false};
};

LocalStatusReport build_local_status(const SecuritySettings& settings);

std::string local_status_to_json(const LocalStatusReport& report);
