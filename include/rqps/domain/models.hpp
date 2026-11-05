#pragma once

#include "rqp/security/validator_core.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace rqps {

struct PackageItem {
    std::string system;
    std::string name;
    std::string version;
};

struct Inventory {
    std::vector<PackageItem> packages;
};

struct DiscoverSummary {
    std::filesystem::path canonicalPath;
    std::size_t artifactCount{0};
    std::size_t packageCount{0};
    bool truncated{false};
    std::vector<std::string> ecosystems;
    std::vector<std::string> catalogers;
    Inventory inventory;
    std::string sourceType{"directory"};
    std::vector<std::string> warnings;
};

struct PolicyEvaluation {
    std::vector<ValidationFinding> activeFindings;
    std::vector<ValidationFinding> ignoredFindings;
    bool failed{false};
    int suggestedExitCode{0};
};

struct AuditReport {
    std::optional<DiscoverSummary> discovery;
    std::string discoverJson;
    std::vector<ValidationFinding> rawFindings;
    PolicyEvaluation policy;
};

struct DoctorReport {
    bool syftOk{false};
    std::string syftPath;
    bool hasBlockingIssues{false};
    std::vector<std::string> lines;
    std::vector<std::string> warnings;
};

}  // namespace rqps
