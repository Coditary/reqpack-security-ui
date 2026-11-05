#include "rqps/local_status.h"

#include "rqp/security/security_config.h"
#include "rqp/security/vulnerability_database.h"

#include <filesystem>
#include <sstream>

namespace {

void append_line(LocalStatusReport& report, const std::string& line) {
    report.lines.push_back(line);
}

}  // namespace

LocalStatusReport build_local_status(const SecuritySettings& settings) {
    LocalStatusReport report;

    append_line(report, "security enabled: " + std::string(settings.security.enabled ? "yes" : "no"));
    append_line(report, "onUnsafe: " + to_string(settings.security.onUnsafe));
    append_line(report, "strictEcosystemMapping: " + std::string(settings.security.strictEcosystemMapping ? "yes" : "no"));
    append_line(report, "osvDatabasePath: " + settings.security.osvDatabasePath);
    append_line(report, "osvFeedUrl: " + settings.security.osvFeedUrl);
    append_line(report, "osvRefreshMode: " + to_string(settings.security.osvRefreshMode));

    if (!std::filesystem::exists(settings.security.osvDatabasePath)) {
        report.warnings.push_back("vulnerability database path does not exist yet");
        report.hasBlockingIssues = true;
    }

    VulnerabilityDatabase database(settings);
    if (!database.ensureReady()) {
        report.warnings.push_back("vulnerability database could not be opened");
        report.hasBlockingIssues = true;
    } else {
        append_line(report, "database ready: yes");
        append_line(report, "database has advisories: " + std::string(database.hasAdvisories() ? "yes" : "no"));
        if (!database.hasAdvisories()) {
            report.warnings.push_back("database is empty; run: rqps db sync");
            report.hasBlockingIssues = true;
        }
        if (const auto backend = database.getSyncState("backend"); backend.has_value()) {
            append_line(report, "sync backend: " + backend.value());
        }
        if (const auto lastSync = database.getSyncState("last_successful_sync_at"); lastSync.has_value()) {
            append_line(report, "last successful sync (epoch): " + lastSync.value());
        }
    }

    if (!settings.security.osvEcosystemMap.empty()) {
        append_line(report, "configured osvEcosystemMap:");
        for (const auto& [system, ecosystem] : settings.security.osvEcosystemMap) {
            append_line(report, "  " + system + " -> " + ecosystem);
        }
    }

    return report;
}

std::string local_status_to_json(const LocalStatusReport& report) {
    std::ostringstream json;
    json << "{\"hasBlockingIssues\":" << (report.hasBlockingIssues ? "true" : "false") << ",\"lines\":[";
    for (std::size_t index = 0; index < report.lines.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        json << '\"';
        for (char c : report.lines[index]) {
            if (c == '\"' || c == '\\') {
                json << '\\';
            }
            json << c;
        }
        json << '\"';
    }
    json << "],\"warnings\":[";
    for (std::size_t index = 0; index < report.warnings.size(); ++index) {
        if (index > 0) {
            json << ',';
        }
        json << '\"';
        for (char c : report.warnings[index]) {
            if (c == '\"' || c == '\\') {
                json << '\\';
            }
            json << c;
        }
        json << '\"';
    }
    json << "]}";
    return json.str();
}
