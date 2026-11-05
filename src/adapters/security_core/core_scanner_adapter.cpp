#include "rqps/ports/i_scanner.hpp"

#include "rqps/inventory_json.h"
#include "rqps/scan_service.h"

#include <boost/graph/graph_traits.hpp>

namespace rqps {

Inventory inventory_from_graph(const Graph& graph) {
    Inventory inventory;
    for (auto [vertex, end] = boost::vertices(graph); vertex != end; ++vertex) {
        const Package& package = graph[*vertex];
        inventory.packages.push_back(PackageItem{package.system, package.name, package.version});
    }
    return inventory;
}

Graph graph_from_inventory(const Inventory& inventory, std::string& error) {
    error.clear();
    Graph graph;
    for (const PackageItem& item : inventory.packages) {
        Package package;
        package.action = ActionType::INSTALL;
        package.system = item.system;
        package.name = item.name;
        package.version = item.version;
        boost::add_vertex(package, graph);
    }
    if (inventory.packages.empty()) {
        error = "no packages in inventory";
    }
    return graph;
}

Result<std::vector<ValidationFinding>> SecurityCoreScanner::scanInventory(const SecuritySettings& settings,
                                                                            const Inventory& inventory) {
    std::string parseError;
    const Graph graph = graph_from_inventory(inventory, parseError);
    if (!parseError.empty()) {
        return Result<std::vector<ValidationFinding>>::fail(
            RqpsError::make(ErrorCode::InvalidInventoryJson, parseError));
    }

    const InventoryScanResult scanned = scan_inventory_graph(settings, graph);
    if (scanned.error.has_value()) {
        return Result<std::vector<ValidationFinding>>::fail(
            RqpsError::make(ErrorCode::NoPackages, scanned.error.value()));
    }
    return Result<std::vector<ValidationFinding>>::ok(scanned.findings);
}

}  // namespace rqps
