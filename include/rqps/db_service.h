#pragma once

#include "rqp/security/security_settings.h"
#include "rqp/security/validator_core.h"

#include <set>
#include <string>
#include <vector>

std::set<std::string> rqps_default_sync_ecosystems();

std::vector<ValidationFinding> rqps_db_sync(SecuritySettings settings, const std::set<std::string>& ecosystems);
