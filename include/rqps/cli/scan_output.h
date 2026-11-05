#pragma once

#include "rqps/application/policy_evaluator.hpp"
#include "rqps/domain/models.hpp"
#include "rqps/scan_service.h"

#include "rqp/security/security_settings.h"
#include "rqp/security/validator_core.h"

#include <iostream>
#include <string>
#include <vector>

struct ScanOutputOptions {
    bool json{false};
    bool quiet{false};
    bool failOnFindings{true};
};

int print_scan_results(const SecuritySettings& settings, const std::vector<ValidationFinding>& rawFindings,
                       const ScanOutputOptions& options, std::ostream& output = std::cout);

int print_policy_evaluation(const rqps::PolicyEvaluation& evaluation, const ScanOutputOptions& options,
                            std::ostream& output = std::cout);

inline std::vector<ValidationFinding> apply_policy(const SecuritySettings& settings,
                                                   const std::vector<ValidationFinding>& findings) {
    return rqps::evaluate_policy(settings, findings, false).activeFindings;
}
