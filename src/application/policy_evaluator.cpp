#include "rqps/application/policy_evaluator.hpp"

#include "rqps/scan_service.h"

namespace rqps {

PolicyEvaluation evaluate_policy(const SecuritySettings& settings, const std::vector<ValidationFinding>& rawFindings,
                                 bool failOnFindings) {
    PolicyEvaluation evaluation;
    const std::vector<ValidationFinding> policyApplied =
        validator_apply_rules(rawFindings, settings.security.allowVulnerabilityIds, settings.security.ignoreVulnerabilityIds);
    evaluation.activeFindings = ::filter_vulnerability_findings(policyApplied);

    const ValidationPolicy policy = validator_policy_from_config(settings);
    evaluation.failed = failOnFindings && validator_findings_exceed_threshold(evaluation.activeFindings, policy);
    evaluation.suggestedExitCode = evaluation.failed ? 1 : 0;
    return evaluation;
}

}  // namespace rqps
