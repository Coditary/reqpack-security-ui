#include "rqps/rqps_config.h"

#include "rqp/security/security_settings.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstdlib>
#include <fstream>

namespace {

std::string expand_home(std::string value) {
    if (value.size() >= 2 && value.rfind("~/", 0) == 0) {
        if (const char* home = std::getenv("HOME")) {
            return std::string(home) + value.substr(1);
        }
    }
    return value;
}

UnsafeAction parse_unsafe_action(const std::string& value) {
    if (value == "abort") {
        return UnsafeAction::ABORT;
    }
    if (value == "prompt") {
        return UnsafeAction::PROMPT;
    }
    return UnsafeAction::CONTINUE;
}

SeverityLevel parse_severity_level(const std::string& value) {
    if (value == "critical") {
        return SeverityLevel::CRITICAL;
    }
    if (value == "high") {
        return SeverityLevel::HIGH;
    }
    if (value == "medium") {
        return SeverityLevel::MEDIUM;
    }
    if (value == "low") {
        return SeverityLevel::LOW;
    }
    return SeverityLevel::UNASSIGNED;
}

OsvRefreshMode parse_refresh_mode(const std::string& value) {
    if (value == "always") {
        return OsvRefreshMode::ALWAYS;
    }
    if (value == "periodic") {
        return OsvRefreshMode::PERIODIC;
    }
    return OsvRefreshMode::MANUAL;
}

void apply_default_ecosystem_map(SecurityConfig& security) {
    if (security.osvEcosystemMap.empty()) {
        security.osvEcosystemMap = {{"npm", "npm"},
                                    {"pip", "PyPI"},
                                    {"go", "Go"},
                                    {"cargo", "crates.io"},
                                    {"maven", "Maven"},
                                    {"gem", "RubyGems"},
                                    {"composer", "Packagist"},
                                    {"nuget", "NuGet"},
                                    {"apt", "Debian"},
                                    {"dnf", "RPM"},
                                    {"apk", "Alpine"}};
    }
}

void apply_json_tree(RqpsConfig& config, const boost::property_tree::ptree& tree) {
    SecurityConfig& security = config.settings.security;
    if (const auto enabled = tree.get_optional<bool>("enabled")) {
        security.enabled = enabled.value();
    }
    if (const auto autoFetch = tree.get_optional<bool>("autoFetch")) {
        security.autoFetch = autoFetch.value();
    }
#if defined(RQPS_UI_HAVE_SECURITY_PRESET)
    if (const auto preset = tree.get_optional<std::string>("preset")) {
        security.preset = preset.value();
        apply_security_preset(security, security.preset);
    }
#endif
    if (const auto dbPath = tree.get_optional<std::string>("osvDatabasePath")) {
        security.osvDatabasePath = expand_home(dbPath.value());
    }
    if (const auto feedUrl = tree.get_optional<std::string>("osvFeedUrl")) {
        security.osvFeedUrl = feedUrl.value();
    }
    if (const auto refresh = tree.get_optional<std::string>("osvRefreshMode")) {
        security.osvRefreshMode = parse_refresh_mode(refresh.value());
    }
#if defined(RQPS_UI_HAVE_AUDIT_FAIL_ON_FINDINGS)
    if (const auto failOnFindings = tree.get_optional<bool>("auditFailOnFindings")) {
        security.auditFailOnFindings = failOnFindings.value();
    }
#endif
    if (const auto onUnsafe = tree.get_optional<std::string>("onUnsafe")) {
        security.onUnsafe = parse_unsafe_action(onUnsafe.value());
    }
    if (const auto severity = tree.get_optional<std::string>("severityThreshold")) {
        security.severityThreshold = parse_severity_level(severity.value());
    }
    if (const auto score = tree.get_optional<double>("scoreThreshold")) {
        security.scoreThreshold = score.value();
    }
    if (const auto overlay = tree.get_optional<std::string>("osvOverlayPath")) {
        security.osvOverlayPath = expand_home(overlay.value());
    }
    if (const auto strict = tree.get_optional<bool>("strictEcosystemMapping")) {
        security.strictEcosystemMapping = strict.value();
    }

    if (const auto map = tree.get_child_optional("osvEcosystemMap")) {
        for (const auto& [key, value] : map.value()) {
            security.osvEcosystemMap[key] = value.get_value<std::string>();
        }
    }

    if (const auto ignore = tree.get_child_optional("ignoreVulnerabilityIds")) {
        security.ignoreVulnerabilityIds.clear();
        for (const auto& [_, value] : ignore.value()) {
            security.ignoreVulnerabilityIds.push_back(value.get_value<std::string>());
        }
    }
    if (const auto allow = tree.get_child_optional("allowVulnerabilityIds")) {
        security.allowVulnerabilityIds.clear();
        for (const auto& [_, value] : allow.value()) {
            security.allowVulnerabilityIds.push_back(value.get_value<std::string>());
        }
    }

    if (const auto discoverEnabled = tree.get_optional<bool>("discoverEnabled")) {
        config.discover.enabled = discoverEnabled.value();
    }
    if (const auto syftPath = tree.get_optional<std::string>("syftPath")) {
        config.discover.syftExecutable = syftPath.value();
    }
    if (const auto maxPackages = tree.get_optional<std::size_t>("discoverMaxPackages")) {
        config.discover.maxPackages = maxPackages.value();
    }
    if (const auto roots = tree.get_child_optional("discoverAllowedRoots")) {
        config.discover.allowedRoots.clear();
        for (const auto& [_, entry] : roots.value()) {
            config.discover.allowedRoots.push_back(expand_home(entry.get_value<std::string>()));
        }
    }

    security.indexPath = security.osvDatabasePath;
}

void load_json_file_if_exists(RqpsConfig& config, const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return;
    }
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(path.string(), tree);
    apply_json_tree(config, tree);
}

std::optional<std::filesystem::path> find_project_config(const std::filesystem::path& start) {
    std::error_code error;
    std::filesystem::path current = std::filesystem::absolute(start, error);
    if (error) {
        return std::nullopt;
    }
    while (true) {
        const std::filesystem::path candidate = current / ".rqps.json";
        if (std::filesystem::exists(candidate, error)) {
            return candidate;
        }
        if (!current.has_parent_path() || current == current.parent_path()) {
            break;
        }
        current = current.parent_path();
    }
    return std::nullopt;
}

}  // namespace

std::filesystem::path rqps_default_config_dir() {
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
        return std::filesystem::path(xdg) / "rqps";
    }
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".config" / "rqps";
    }
    return std::filesystem::path(".config") / "rqps";
}

std::filesystem::path rqps_default_data_dir() {
    if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(xdg) / "rqps";
    }
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".local" / "share" / "rqps";
    }
    return std::filesystem::path(".local") / "share" / "rqps";
}

RqpsConfig load_rqps_config(const std::filesystem::path& explicitConfigPath) {
    RqpsConfig config;
    config.settings = default_security_settings();
    config.settings.security.enabled = true;
    config.settings.security.autoFetch = true;
    config.settings.security.osvRefreshMode = OsvRefreshMode::MANUAL;
    config.settings.interaction.interactive = false;
    config.settings.security.osvDatabasePath = (rqps_default_data_dir() / "osv").string();
    config.settings.security.osvFeedUrl = "https://storage.googleapis.com/osv-vulnerabilities";
    apply_default_ecosystem_map(config.settings.security);

    config.discover.enabled = true;
    config.discover.enforceAllowedRoots = false;
    config.discover.syftExecutable = "syft";

    const std::filesystem::path globalPath =
        explicitConfigPath.empty() ? rqps_default_config_dir() / "config.json" : explicitConfigPath;
    config.globalConfigPath = globalPath;
    load_json_file_if_exists(config, globalPath);

    if (config.settings.security.osvDatabasePath.empty()) {
        config.settings.security.osvDatabasePath = (rqps_default_data_dir() / "osv").string();
    }
    config.settings.security.indexPath = config.settings.security.osvDatabasePath;
    return config;
}

RqpsConfig merge_project_config(RqpsConfig base, const std::filesystem::path& projectDirectory) {
    const auto projectConfig = find_project_config(projectDirectory);
    if (projectConfig.has_value()) {
        base.projectConfigPath = projectConfig;
        load_json_file_if_exists(base, projectConfig.value());
    }
    return base;
}
