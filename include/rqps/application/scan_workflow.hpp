#pragma once

#include "rqps/domain/error.hpp"
#include "rqps/ports/i_scanner.hpp"

#include "rqp/security/security_settings.h"

#include <string>

namespace rqps {

Result<std::string> scan_inventory_json_report(const SecuritySettings& settings, const std::string& inputJson,
                                               IVulnerabilityScanner& scanner);

}  // namespace rqps
