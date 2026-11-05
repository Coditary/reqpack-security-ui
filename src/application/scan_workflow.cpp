#include "rqps/application/scan_workflow.hpp"

#include "rqps/inventory_json.h"
#include "rqps/scan_service.h"

namespace rqps {

Result<std::string> scan_inventory_json_report(const SecuritySettings& settings, const std::string& inputJson,
                                               IVulnerabilityScanner& scanner) {
    std::string parseError;
    const Graph graph = graph_from_inventory_json(inputJson, parseError);
    if (!parseError.empty()) {
        return Result<std::string>::fail(RqpsError::make(ErrorCode::InvalidInventoryJson, parseError));
    }

    const Inventory inventory = inventory_from_graph(graph);
    const Result<std::vector<ValidationFinding>> scanned = scanner.scanInventory(settings, inventory);
    if (!scanned.has_value()) {
        return Result<std::string>::fail(scanned.error());
    }
    return Result<std::string>::ok(findings_to_json(scanned.value()));
}

}  // namespace rqps
