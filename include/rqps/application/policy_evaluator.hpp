#pragma once

#include "rqps/domain/models.hpp"

#include "rqp/security/security_settings.h"
#include "rqp/security/validator_core.h"

#include <vector>

namespace rqps {

PolicyEvaluation evaluate_policy(const SecuritySettings& settings, const std::vector<ValidationFinding>& rawFindings,
                                 bool failOnFindings);

}  // namespace rqps
