#include "rqps/application/service_registry.hpp"

namespace rqps {

SecurityCoreScanner& default_scanner() {
    static SecurityCoreScanner scanner;
    return scanner;
}

AuditWorkflow& default_audit_workflow() {
    static AuditWorkflow workflow(default_process_runner(), default_scanner());
    return workflow;
}

}  // namespace rqps
