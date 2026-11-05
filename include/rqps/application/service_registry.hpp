#pragma once

#include "rqps/application/audit_workflow.hpp"
#include "rqps/ports/i_scanner.hpp"

namespace rqps {

SecurityCoreScanner& default_scanner();
AuditWorkflow& default_audit_workflow();

}  // namespace rqps
