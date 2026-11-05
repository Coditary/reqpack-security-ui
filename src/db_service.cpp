#include "rqps/db_service.h"

#include "rqp/security/vulnerability_database.h"
#include "rqp/security/vulnerability_sync_service.h"

std::set<std::string> rqps_default_sync_ecosystems() {
    return {"npm", "PyPI", "Go", "crates.io", "Maven", "RubyGems", "Packagist", "NuGet", "Debian", "RPM", "Alpine"};
}

std::vector<ValidationFinding> rqps_db_sync(SecuritySettings settings, const std::set<std::string>& ecosystems) {
    settings.security.enabled = true;
    settings.security.autoFetch = true;
    settings.security.osvRefreshMode = OsvRefreshMode::ALWAYS;
    settings.interaction.interactive = false;

    VulnerabilityDatabase database(settings);
    database.ensureReady();

    VulnerabilitySyncService syncService(&database, nullptr, settings, ecosystems);
    return syncService.ensureReady();
}
