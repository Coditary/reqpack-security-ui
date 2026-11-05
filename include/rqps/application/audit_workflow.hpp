#pragma once

#include "rqps/domain/config_model.hpp"
#include "rqps/domain/error.hpp"
#include "rqps/domain/models.hpp"
#include "rqps/ports/i_process_runner.hpp"
#include "rqps/ports/i_scanner.hpp"

#include <filesystem>

namespace rqps {

struct AuditWorkflowOptions {
    bool dryRun{false};
    bool failOnFindings{true};
};

class AuditWorkflow {
public:
    AuditWorkflow(IProcessRunner& processRunner, IVulnerabilityScanner& scanner);

    Result<AuditReport> run(const RqpsConfig& config, const std::filesystem::path& projectPath,
                            const AuditWorkflowOptions& options) const;

private:
    IProcessRunner& processRunner_;
    IVulnerabilityScanner& scanner_;
};

}  // namespace rqps
