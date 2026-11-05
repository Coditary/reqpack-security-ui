#include "rqps/application/audit_workflow.hpp"

#include "rqps/application/policy_evaluator.hpp"
#include "rqps/syft_discover_service.h"

namespace rqps {

AuditWorkflow::AuditWorkflow(IProcessRunner& processRunner, IVulnerabilityScanner& scanner)
    : processRunner_(processRunner), scanner_(scanner) {}

Result<AuditReport> AuditWorkflow::run(const RqpsConfig& config, const std::filesystem::path& projectPath,
                                       const AuditWorkflowOptions& options) const {
    const Result<DiscoverSummary> discovered = syft_discover_summary(config.discover, projectPath, processRunner_);
    if (!discovered.has_value()) {
        return Result<AuditReport>::fail(discovered.error());
    }

    AuditReport report;
    report.discovery = discovered.value();
    report.discoverJson = discover_summary_to_json(discovered.value());

    if (options.dryRun) {
        report.policy.suggestedExitCode = 0;
        return Result<AuditReport>::ok(std::move(report));
    }

    const Result<std::vector<ValidationFinding>> scanned =
        scanner_.scanInventory(config.settings, discovered.value().inventory);
    if (!scanned.has_value()) {
        return Result<AuditReport>::fail(scanned.error());
    }

    report.rawFindings = scanned.value();
    report.policy = evaluate_policy(config.settings, report.rawFindings, options.failOnFindings);
    return Result<AuditReport>::ok(std::move(report));
}

}  // namespace rqps
