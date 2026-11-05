#pragma once

#include "rqps/domain/error.hpp"
#include "rqps/domain/models.hpp"

#include "rqp/security/security_types.h"

#include "rqp/security/security_settings.h"
#include "rqp/security/validator_core.h"

#include <filesystem>

namespace rqps {

class IVulnerabilityScanner {
public:
    virtual ~IVulnerabilityScanner() = default;

    virtual Result<std::vector<ValidationFinding>> scanInventory(const SecuritySettings& settings,
                                                                 const Inventory& inventory) = 0;
};

class SecurityCoreScanner final : public IVulnerabilityScanner {
public:
    Result<std::vector<ValidationFinding>> scanInventory(const SecuritySettings& settings,
                                                         const Inventory& inventory) override;
};

Inventory inventory_from_graph(const Graph& graph);
Graph graph_from_inventory(const Inventory& inventory, std::string& error);

}  // namespace rqps
