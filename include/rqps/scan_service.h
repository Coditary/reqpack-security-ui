#pragma once

#include "rqp/security/security_settings.h"
#include "rqp/security/validator_core.h"

#include <optional>
#include <string>
#include <vector>

struct ScanResult {
    std::string json;
    std::optional<std::string> error;
};

struct InventoryScanResult {
    std::vector<ValidationFinding> findings;
    std::optional<std::string> error;
};

InventoryScanResult scan_inventory_graph(const SecuritySettings& settings, Graph graph);

ScanResult run_inventory_scan(const SecuritySettings& settings, const std::string& inputJson);

std::string findings_to_json(const std::vector<ValidationFinding>& findings);

std::vector<ValidationFinding> filter_vulnerability_findings(const std::vector<ValidationFinding>& findings);
